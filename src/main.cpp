// src/main.cpp - 智慧門鎖主程式（PIR 人體感測器版）
#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "pir_sensor.h"
#include "face_recognition.h"
#include "face_database.h"
#include "face_manager.h"
#include "oled_ui.h"
#include "battery.h"
#include "keypad.h"
#include "fingerprint.h"
#include "audio.h"
#include "relay.h"
#include "wifi_mgr.h"
#include "weather.h"
#include "telegram_bot.h"

// ===== 全域物件 =====
OledUI                ui;
FaceRecognitionSystem faceSystem;
FaceDatabase          faceDB;
FaceManager           faceMgr(faceSystem, faceDB, ui);
BatteryMonitor        battery;
PIRSensor             pir;

HardwareSerial       fpSerial(1);
Adafruit_Fingerprint finger(&fpSerial);

// ===== 執行時狀態 =====
String       currentPassword = DEFAULT_PASSWORD;
int          failCount       = 0;
SystemState  currentState    = STATE_SLEEP;
String       inputBuffer     = "";
unsigned long unlockTimestamp = 0;
String       lastUnlockName  = "";

WeatherInfo  weatherCache;
unsigned long lastWeatherUpdate = 0;

unsigned long lastPIRCheck     = 0;
unsigned long lastActivityTime = 0;
unsigned long lastWeatherAnnounceTime = 0;

extern String pendingEnrollName;
extern bool   pendingEnroll;

// ===== 前向宣告 =====
void handleSleep();
void handleIdle();
void handleUnlocked();
void handleAlarm();
void handleFaceMgmt();
void handleKeyInput(char key);
void handleFaceCheck();
void successUnlock(const String& name, camera_fb_t* photoFb);
void failedAttempt();
void setLED(bool green, bool red);
void markActivity();
void checkPendingEnrollWrapper(camera_fb_t* fb);
void startFingerprintEnrollment();
void startFingerprintVerify();

void markActivity() {
    lastActivityTime = millis();
}

void setLED(bool green, bool red) {
    uint8_t status = pcf8574_read(PCF_STATUS_ADDR);
    status &= ~(1 << LED_GREEN_P);
    status &= ~(1 << LED_RED_P);
    if (!green) status |= (1 << LED_GREEN_P);
    if (!red)   status |= (1 << LED_RED_P);
    // 保護 PIR_IN_P 腳位，永遠保持為輸入模式（HIGH）
    status |= (1 << PIR_IN_P);
    pcf8574_write(PCF_STATUS_ADDR, status);
}

void successUnlock(const String& name, camera_fb_t* photoFb) {
    Serial.printf("🔓 解鎖！[%s]\n", name.c_str());
    failCount = 0;
    lastUnlockName = name;
    currentState = STATE_UNLOCKED;
    unlockTimestamp = millis();
    markActivity();
    unlockDoor(UNLOCK_DURATION_MS);
    playSoundAsync(SOUND_UNLOCK);
    setLED(true, false);
    String weatherMsg = getWeatherMessage(weatherCache);
    ui.showUnlocked(name, weatherMsg);
    sendTelegramMessage("✅ 解鎖：" + name + "\n" + getCurrentDateTime() + "\n" + weatherMsg);
}

void failedAttempt() {
    failCount++;
    markActivity();
    playSoundAsync(SOUND_DENY);
    setLED(false, true);
    delay(500);
    setLED(false, false);
    ui.showDenied(failCount, MAX_FAIL_ATTEMPTS);
    delay(1500);
    if (failCount >= MAX_FAIL_ATTEMPTS) {
        currentState = STATE_ALARM;
        sendTelegramMessage("🚨 警報！連續失敗 " + String(MAX_FAIL_ATTEMPTS) + " 次\n" + getCurrentDateTime());
    }
}

void handleKeyInput(char key) {
    markActivity();
    playSoundAsync(SOUND_BEEP);
    Serial.printf("🔢 按下按鍵: '%c'\n", key);

    if (key == 'A') {
        if (inputBuffer == ADMIN_PASSWORD) {
            inputBuffer = "";
            currentState = STATE_FACE_MGMT;
            Serial.println("→ 管理模式");
        } else {
            ui.showMessage("Access Denied", "Wrong pwd");
            delay(1500);
        }
        inputBuffer = "";
        return;
    }

    if (key == '#') {
        if (inputBuffer == currentPassword) {
            successUnlock("密碼", nullptr);
        } else {
            failedAttempt();
        }
        inputBuffer = "";
        ui.showPasswordInput(0);
    } else if (key == '*') {
        inputBuffer = "";
        ui.showPasswordInput(0);
    } else if (key == 'D' && inputBuffer.length() > 0) {
        inputBuffer.remove(inputBuffer.length() - 1);
        ui.showPasswordInput(inputBuffer.length());
    } else if (key == 'C') {
        startFingerprintVerify();
    } else if (isDigit(key)) {
        inputBuffer += key;
        ui.showPasswordInput(inputBuffer.length());
    }
}

void handleSleep() {
    if (millis() - lastPIRCheck < 500) return;
    lastPIRCheck = millis();

    // 門外 PIR 檢測
    if (pir.isOutsideDetected()) {
        Serial.println("👤 門外偵測到人體，喚醒系統");
        ui.showMessage("Someone outside!", "Wake up");
        playSoundAsync(SOUND_BEEP);
        currentState = STATE_IDLE;
        lastActivityTime = millis();
        return;
    }

    // 門內 PIR 檢測（喚醒系統以進行天氣播報）
    if (pir.isInsideDetected()) {
        Serial.println("🏠 門內偵測到人體，喚醒系統");
        ui.showMessage("Someone inside!", "Wake up");
        currentState = STATE_IDLE;
        lastActivityTime = millis();
    }
}

void handleIdle() {
    if (millis() - lastPIRCheck >= 500) {
        lastPIRCheck = millis();
        
        // 門內 PIR 檢查
        if (pir.isInsideDetected()) {
        
            unsigned long now = millis();
            if (now - lastWeatherAnnounceTime > (PIR_COOLDOWN_SEC * 1000UL)) {
                lastWeatherAnnounceTime = now;
                Serial.println("🏠 門內偵測到人體，播報天氣");
                if (WEATHER_NOTIFY_EN && weatherCache.valid) {
                    ui.showMessage("Weather:", getWeatherMessage(weatherCache));
                    
                    // 組合天氣 TTS 語音訊息
                    String speech = "您好，目前溫度 ";
                    speech += String((int)weatherCache.temp);
                    speech += " 度。";
                    if (weatherCache.rainToday) {
                        speech += "外面有雨，出門請記得帶傘喔！";
                    } else {
                        speech += "天氣不錯，祝您出門平安！";
                    }
                    // 執行語音播報
                    playWeatherTTS(speech);
                }
            }
        }

        // 門外 PIR 檢查進入休眠
        bool outsideDetected = pir.isOutsideDetected();
        if (!outsideDetected && (millis() - lastActivityTime) / 1000 >= SLEEP_TIMEOUT_SEC) {
            Serial.printf("💤 %d 秒無人就入休眠\n", SLEEP_TIMEOUT_SEC);
            ui.showMessage("Standby...", "Sleep");
            delay(800);
            ui.display.ssd1306_command(SSD1306_DISPLAYOFF);
            setLED(false, false);
            inputBuffer = "";
            currentState = STATE_SLEEP;
            return;
        } else if (outsideDetected) {
            markActivity();
        }
    }

    static unsigned long lastDisplayUpdate = 0;
    if (millis() - lastDisplayUpdate > 1000) {
        lastDisplayUpdate = millis();
        ui.display.ssd1306_command(SSD1306_DISPLAYON);
        auto batt = battery.getStatus();
        struct tm t;
        getLocalTime(&t);
        char timeStr[6];
        strftime(timeStr, 6, "%H:%M", &t);
        ui.showIdle(String(timeStr), weatherCache.temp, weatherCache.rainToday, batt.percentage, batt.charging);
    }

    char key = scanKeypad(PCF_KEYPAD_ADDR);
    if (key) handleKeyInput(key);

    if (FINGERPRINT_EN) {
        int fpId = verifyFingerprint();
        if (fpId >= 0) {
            successUnlock("指紋 #" + String(fpId), nullptr);
        } else if (fpId == -1) {
            failedAttempt();
        }
    }

    static unsigned long lastFaceCheck = 0;
    if (millis() - lastFaceCheck > FACE_SCAN_INTERVAL_MS) {
        lastFaceCheck = millis();
        handleFaceCheck();
    }
}

void handleFaceCheck() {
    camera_fb_t* fb = faceSystem.capture();
    if (!fb) return;

    if (pendingEnroll) {
        checkPendingEnrollWrapper(fb);
        faceSystem.returnFrame(fb);
        return;
    }

    ui.showVerifying("Face Recognition");
    auto result = faceSystem.process(fb);

    if (result.recognized) {
        markActivity();
        successUnlock(result.name, fb);
    } else if (result.face_detected && STRANGER_ALERT_EN) {
        static unsigned long lastAlert = 0;
        if (millis() - lastAlert > 30000) {
            lastAlert = millis();
            sendTelegramPhoto(fb, "⚠️ 陌生人警報！");
        }
    }
    faceSystem.returnFrame(fb);
}

void checkPendingEnrollWrapper(camera_fb_t* fb) {
    if (!pendingEnroll || !fb) return;
    static unsigned long pendingStart = 0;
    static int enrollCount = 0;
    if (pendingStart == 0) {
        pendingStart = millis();
        Serial.printf("[登錄] 開始：%s\n", pendingEnrollName.c_str());
    }
    if (millis() - pendingStart > 30000) {
        pendingEnroll = false;
        pendingStart = 0;
        sendTelegramMessage("⏰ 登錄逾時");
        return;
    }
    bool ok = faceSystem.enroll(fb, pendingEnrollName);
    if (ok) {
        enrollCount++;
        if (enrollCount >= FACE_ENROLL_SAMPLES) {
            pendingEnroll = false;
            pendingStart = 0;
            enrollCount = 0;
            sendTelegramMessage("✅ 登錄完成：" + pendingEnrollName);
        }
    }
}

void handleUnlocked() {
    updateRelay();
    int remaining = getUnlockRemainingSeconds();
    if (remaining < 0) remaining = 0;
    static int lastRemaining = -1;
    if (remaining != lastRemaining) {
        lastRemaining = remaining;
        ui.showUnlocked(lastUnlockName, "Lock in " + String(remaining) + "s");
    }
    if (!isDoorUnlocked()) {
        setLED(false, false);
        currentState = STATE_IDLE;
        Serial.println("🔒 已自動鎖門");
    }
}

void handleAlarm() {
    ui.showAlarm();
    static unsigned long lastBeep = 0;
    if (millis() - lastBeep > 2000) {
        lastBeep = millis();
        playSoundAsync(SOUND_ALARM);
        setLED(false, (millis() / 350) % 2);
    }
    char key = scanKeypad(PCF_KEYPAD_ADDR);
    if (!key) return;
    markActivity();
    if (key == '*') {
        inputBuffer = "";
    } else if (key == '#') {
        if (inputBuffer == currentPassword || inputBuffer == ADMIN_PASSWORD) {
            failCount = 0;
            currentState = STATE_IDLE;
            setLED(false, false);
            sendTelegramMessage("警報已解除");
        }
        inputBuffer = "";
    } else if (isDigit(key)) {
        inputBuffer += key;
    }
}

void handleFaceMgmt() {
    ui.showFaceMenu(faceSystem.getCount());
    char key = waitForKey(PCF_KEYPAD_ADDR, 15000);

    if (key == '1') {
        ui.showMessage("Add Face", "Use Telegram");
        sendTelegramMessage("📷 請使用 /face_enroll 姓名");
    } else if (key == '2') {
        startFingerprintEnrollment();
    } else if (key == '3') {
        auto list = faceSystem.getList();
        if (list.empty()) ui.showMessage("No faces", "Empty");
        else ui.showList("Faces", list);
    } else if (key == '4') {
        if (waitForKey(PCF_KEYPAD_ADDR, 5000) == '#') {
            faceSystem.deleteAll();
            ui.showMessage("Done", "All deleted");
            playSoundAsync(SOUND_DENY);
        }
    }
    if (key == '*' || key == 0) currentState = STATE_IDLE;
}

void startFingerprintEnrollment() {
    Serial.println("═══ 指紋登錄 ═══");
    
    // 1. 先找出目前空著的 ID 是幾號
    int freeId = getNextFreeFingerprintID();
    
    if (freeId == -1) {
        ui.showMessage("FP Full", "Delete some");
        sendTelegramMessage("❌ 指紋容量已滿 (127 筆)，請先刪除部分指紋！");
        delay(3000);
        return; // 容量滿了就直接退出
    }

    // 2. 顯示即將存入的 ID
    ui.showMessage("FP Enroll", "ID: " + String(freeId));
    sendTelegramMessage("👆 開始指紋登錄 (將配發專屬 ID: " + String(freeId) + ")...");
    
    // 3. 把找到的空位 ID 傳進去登錄
    if (enrollFingerprint(freeId)) {
        ui.showMessage("FP OK!", "ID: " + String(freeId));
        sendTelegramMessage("✅ 指紋登錄成功！您的專屬 ID 為：" + String(freeId));
    } else {
        ui.showMessage("FP Fail", "Try again");
        sendTelegramMessage("❌ 指紋登錄失敗，請重試");
    }
    
    delay(3000);
}

void startFingerprintVerify() {
    Serial.println("═══ 指紋偵測（7秒） ═══");
    ui.showMessage("FP Verify", "Place finger");
    
    unsigned long startTime = millis();
    while (millis() - startTime < 7000) {
        // 使用安全的封裝函數
        int fpId = verifyFingerprint();
        if (fpId >= 0) {
            Serial.printf("✅ 指紋成功 ID: %d\n", fpId);
            successUnlock("指紋 #" + String(fpId), nullptr);
            return;
        }
        delay(100);
    }
    ui.showMessage("FP Timeout", "Try again");
    delay(1500);
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("╔═══════════════════════╗");
    Serial.println("║ 智慧門鎖 V2 啟動中     ║");
    Serial.println("╚═══════════════════════╝");

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    ui.begin();
    ui.showMessage("Booting...", "Init");

    pcf8574_init(PCF_KEYPAD_ADDR);
    pcf8574_init(PCF_STATUS_ADDR);

    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);

    battery.begin();
    pir.begin();
    Serial.println("✅ PIR 初始化完成");

    ui.showMessage("Booting...", "Camera");
    faceSystem.initCamera();
    faceDB.begin();
    faceMgr.restoreDatabase();

    ui.showMessage("Booting...", "Fingerprint");
    if (!initFingerprint()) {
        Serial.println("⚠️ 指紋模組未找到");
    }

    initAudio();

    bool wifiOK = connectWiFi();
    if (wifiOK) {
        syncTime();
        weatherCache = getWeather();
    }

    playSound(SOUND_STARTUP);
    setLED(false, false);

    if (wifiOK) {
        sendTelegramMessage("🔐 智慧門鎖 V2 已啟動\nIP: " + WiFi.localIP().toString());
    }

    ui.showMessage("Ready.", "Waiting...");
    delay(1500);
    ui.display.ssd1306_command(SSD1306_DISPLAYOFF);

    currentState = STATE_SLEEP;
    Serial.println("✅ 啟動完成");
}

void loop() {
    maintainWiFi();

    static unsigned long lastBotPoll = 0;
    if (millis() - lastBotPoll > BOT_POLL_MS) {
        lastBotPoll = millis();
        handleTelegramCommands();
        if (currentState == STATE_SLEEP && isDoorUnlocked()) {
            ui.display.ssd1306_command(SSD1306_DISPLAYON);
            currentState = STATE_UNLOCKED;
            markActivity();
        }
    }

    if (millis() - lastWeatherUpdate > WEATHER_UPDATE_MS) {
        lastWeatherUpdate = millis();
        if (WiFi.status() == WL_CONNECTED) weatherCache = getWeather();
    }

    static unsigned long lastBattCheck = 0;
    if (millis() - lastBattCheck > 300000) {
        lastBattCheck = millis();
        auto b = battery.getStatus(false);
        if (b.lowBattery && currentState != STATE_SLEEP) {
            playSoundAsync(SOUND_LOW_BATT);
            sendTelegramMessage("⚡ 電量不足：" + String(b.percentage) + "%");
        }
    }

    switch (currentState) {
        case STATE_SLEEP:      handleSleep();     break;
        case STATE_IDLE:       handleIdle();      break;
        case STATE_UNLOCKED:   handleUnlocked();  break;
        case STATE_ALARM:      handleAlarm();     break;
        case STATE_FACE_MGMT:  handleFaceMgmt(); break;
        default:               handleIdle();      break;
    }
}