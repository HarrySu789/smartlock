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
void processTelegramCommand(const String& text, const String& fromId);
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
    String weatherDisplay = WEATHER_NOTIFY_EN ? weatherMsg : "";
    ui.showUnlocked(name, weatherDisplay);
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
    Serial.printf("══════════════════════════\n");
    Serial.printf("🔢 按下按鍵: '%c' (ASCII: %d)\n", key, (int)key);
    Serial.printf("══════════════════════════\n");

    if (key == 'A') {
        Serial.println("  → 按下 A 鍵（管理模式）");
        Serial.printf("  → 輸入緩衝: \"%s\"\n", inputBuffer.c_str());
        if (inputBuffer == ADMIN_PASSWORD) {
            inputBuffer = "";
            currentState = STATE_FACE_MGMT;
            Serial.println("  → 管理密碼正確，進入管理模式");
        } else {
            ui.showMessage("Access Denied", "Wrong admin pwd");
            Serial.println("  → 管理密碼錯誤");
            delay(1500);
        }
        inputBuffer = "";
        return;
    }

    if (key == '#') {
        Serial.printf("  → 按下 # 鍵（確認）\n");
        Serial.printf("  → 輸入緩衝: \"%s\" vs 密碼: \"%s\"\n", inputBuffer.c_str(), currentPassword.c_str());
        if (inputBuffer == currentPassword) {
            Serial.println("  → 密碼正確！");
            successUnlock("Password", nullptr);
        } else {
            Serial.println("  → 密碼錯誤！");
            failedAttempt();
        }
        inputBuffer = "";
        ui.showPasswordInput(0);
    } else if (key == '*') {
        Serial.println("  → 按下 * 鍵（清除全部）");
        inputBuffer = "";
        ui.showPasswordInput(0);
    } else if (key == 'D') {
        if (inputBuffer.length() > 0) {
            inputBuffer.remove(inputBuffer.length() - 1);
            ui.showPasswordInput(inputBuffer.length());
            Serial.printf("  → 刪除一位，目前輸入: \"%s\"\n", inputBuffer.c_str());
        }
    } else if (key == 'C') {
        Serial.println("  → 按下 C 鍵（指紋偵測 7 秒）");
        startFingerprintVerify();
    } else if (isDigit(key)) {
        inputBuffer += key;
        ui.showPasswordInput(inputBuffer.length());
        Serial.printf("  → 已輸入 %d 位: \"%s\"\n", inputBuffer.length(), inputBuffer.c_str());
    }
}

void handleSleep() {
    if (millis() - lastPIRCheck < 500) return;
    lastPIRCheck = millis();

    if (pir.isOutsideDetected()) {
        Serial.println("👤 門外偵測到人體，喚醒系統");
        ui.showMessage("Someone outside!", "Wake up system");
        playSoundAsync(SOUND_BEEP);
        currentState = STATE_IDLE;
        lastActivityTime = millis();
        Serial.println("✅ 系統已喚醒");
        return;
    }

    if (pir.isInsideDetected()) {
        unsigned long now = millis();
        if (now - lastWeatherAnnounceTime > (PIR_COOLDOWN_SEC * 1000UL)) {
            lastWeatherAnnounceTime = now;
            Serial.println("🏠 門內偵測到人體，播報天氣");
            if (WEATHER_NOTIFY_EN && weatherCache.valid) {
                String weatherMsg = getWeatherMessage(weatherCache);
                ui.showMessage("Weather:", weatherMsg);
            }
        }
    }
}

void handleIdle() {
    if (millis() - lastPIRCheck >= 500) {
        lastPIRCheck = millis();
        bool outsideDetected = pir.isOutsideDetected();
        
        if (pir.isInsideDetected()) {
            unsigned long now = millis();
            if (now - lastWeatherAnnounceTime > (PIR_COOLDOWN_SEC * 1000UL)) {
                lastWeatherAnnounceTime = now;
                Serial.println("🏠 門內偵測到人體，播報天氣");
                if (WEATHER_NOTIFY_EN && weatherCache.valid) {
                    String weatherMsg = getWeatherMessage(weatherCache);
                    ui.showMessage("Weather:", weatherMsg);
                }
            }
        }

        if (!outsideDetected) {
            unsigned long idleSec = (millis() - lastActivityTime) / 1000;
            if (idleSec >= SLEEP_TIMEOUT_SEC) {
                Serial.printf("💤 %lu 秒無人就入休眠\n", (unsigned long)SLEEP_TIMEOUT_SEC);
                ui.showMessage("Standby...", "Entering sleep");
                delay(800);
                ui.display.ssd1306_command(SSD1306_DISPLAYOFF);
                setLED(false, false);
                inputBuffer = "";
                currentState = STATE_SLEEP;
                return;
            }
        } else {
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
            markActivity();
            // OLED 顯示成功的 ID 與分數
            String scoreMsg = "Score: " + String(finger.confidence);
            ui.showMessage("FP Match ID:" + String(fpId), scoreMsg);
            delay(1000); // 暫停 1 秒讓你能在畫面上看清楚分數
            
            successUnlock("Fingerprint #" + String(fpId), nullptr);
            
        } else if (fpId == -1) {
            markActivity();
            // OLED 顯示失敗，順便秀出剛剛判斷的低分數 (如果有的話)
            String failMsg = "Score: " + String(finger.confidence);
            ui.showMessage("FP Denied", failMsg);
            delay(1000); // 暫停 1 秒讓你能在畫面上看清楚分數
            
            failedAttempt();
        }
        // fpId == -2 (沒放好或沒放手指) 則不處理
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
    } else if (result.face_detected) {
        if (STRANGER_ALERT_EN) {
            Serial.println("⚠️ 偵測到陌生人！");
            static unsigned long lastAlert = 0;
            if (millis() - lastAlert > 30000) {
                lastAlert = millis();
                markActivity();
                struct tm t;
                getLocalTime(&t);
                char timeStr[32];
                strftime(timeStr, 32, "%Y/%m/%d %H:%M:%S", &t);
                String caption = "⚠️ 陌生人警報！\n時間: " + String(timeStr);
                sendTelegramPhoto(fb, caption);
            }
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
        enrollCount = 0;
        Serial.printf("[TG 登錄] 開始：%s\n", pendingEnrollName.c_str());
    }
    if (millis() - pendingStart > 30000) {
        pendingEnroll = false;
        pendingStart = 0;
        enrollCount = 0;
        sendTelegramMessage("⏰ 登錄逾時：" + pendingEnrollName + "\n請重新使用 /face_enroll 指令");
        return;
    }
    bool ok = faceSystem.enroll(fb, pendingEnrollName);
    if (ok) {
        enrollCount++;
        Serial.printf("[TG 登錄] %s 第 %d/%d 張\n", pendingEnrollName.c_str(), enrollCount, FACE_ENROLL_SAMPLES);
        if (enrollCount >= FACE_ENROLL_SAMPLES) {
            pendingEnroll = false;
            pendingStart = 0;
            enrollCount = 0;
            sendTelegramMessage("✅ 人臉登錄完成：" + pendingEnrollName + "\n儲 " + String(FACE_ENROLL_SAMPLES) + " 張樣本");
            Serial.printf("[TG 登錄] 完成：%s\n", pendingEnrollName.c_str());
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
        String msg = "Auto-lock in " + String(remaining) + "s";
        String weather = WEATHER_NOTIFY_EN ? getWeatherMessage(weatherCache) : "";
        ui.showUnlocked(lastUnlockName, weather.length() > 0 ? weather : msg);
    }
    if (!isDoorUnlocked()) {
        setLED(false, false);
        currentState = STATE_IDLE;
        lastRemaining = -1;
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
            inputBuffer = "";
            sendTelegramMessage("ℹ️ 警報已解除");
        }
        inputBuffer = "";
    } else if (isDigit(key)) {
        inputBuffer += key;
    }
}

void handleFaceMgmt() {
    ui.showFaceMenu(faceSystem.getCount());
    markActivity();
    char key = waitForKey(PCF_KEYPAD_ADDR, 15000);
    markActivity();

    if (key == '1') {
        ui.showMessage("Add Face", "Use Telegram:\n/face_enroll [name]");
        sendTelegramMessage("📷 請使用 Telegram 登錄：\n/face_enroll 姓名");
        delay(3000);
    } else if (key == '2') {
        startFingerprintEnrollment();
    } else if (key == '3') {
        auto list = faceSystem.getList();
        if (list.empty()) ui.showMessage("No faces", "Database empty");
        else ui.showList("Enrolled Faces", list);
        delay(4000);
    } else if (key == '4') {
        ui.showMessage("Confirm?", "Press # to delete ALL");
        char c = waitForKey(PCF_KEYPAD_ADDR, 5000);
        if (c == '#') {
            faceSystem.deleteAll();
            faceDB.removeAll();
            ui.showMessage("Done", "All faces deleted");
            playSoundAsync(SOUND_DENY);
            delay(2000);
        }
    }
    if (key == '*' || key == 0) currentState = STATE_IDLE;
}

// ===== 指紋登錄（12 次按壓）=====
void startFingerprintEnrollment() {
    Serial.println("═══ 開始指紋登錄（12次） ═══");
    sendTelegramMessage("👆 開始指紋登錄流程（12次按壓）...");
    
    ui.showMessage("FP Enroll", "Step 1:\nPress 12x");
    Serial.println("步驟 1: 請按壓手指 12 次");
    sendTelegramMessage("步驟 1: 請在指紋感測器上按壓手指\n需要按壓 12 次");
    
    int fingerCount = 0;
    const int maxTries = 30;
    const int requiredPresses = 12;
    
    while (fingerCount < requiredPresses) {
        if (finger.getImage() == FINGERPRINT_OK) {
            int p = finger.image2Tz(1);
            if (p == FINGERPRINT_OK) {
                fingerCount++;
                Serial.printf("✅ 第 %d/%d 次取像成功\n", fingerCount, requiredPresses);
                sendTelegramMessage("✅ 第 " + String(fingerCount) + "/" + String(requiredPresses) + " 次成功");
                playSoundAsync(SOUND_BEEP);
            }
        }
        delay(100);
    }
    
    ui.showMessage("FP Enroll", "Creating\nmodel...");
    Serial.println("步驟 2: 建立指紋模型中...");
    sendTelegramMessage("步驟 2: 建立指紋模型...");
    
    int modelResult = finger.createModel();
    if (modelResult == FINGERPRINT_OK) {
        Serial.println("✅ 模型建立成功");
        sendTelegramMessage("✅ 指紋模型建立成功");
        playSoundAsync(SOUND_UNLOCK);
        
        int storeResult = finger.storeModel(1);
        if (storeResult == FINGERPRINT_OK) {
            ui.showMessage("FP Enroll", "Success!\nID: 1");
            Serial.println("✅ 指紋登錄成功！已儲存到 ID 1");
            sendTelegramMessage("✅ 指紋登錄成功！\n已儲存到 ID: 1\n\n您現在可以使用指紋開門了！");
        } else {
            ui.showMessage("FP Error", "Store fail");
            Serial.printf("❌ 儲存失敗，錯誤碼: %d\n", storeResult);
            sendTelegramMessage("❌ 儲存失敗，錯誤碼: " + String(storeResult));
        }
    } else {
        ui.showMessage("FP Error", "Model fail");
        Serial.printf("❌ 模型建立失敗，錯誤碼: %d\n", modelResult);
        sendTelegramMessage("❌ 模型建立失敗，錯誤碼: " + String(modelResult));
    }
    
    delay(3000);
}

// ===== 指紋偵測（7 秒）=====
void startFingerprintVerify() {
    Serial.println("═══ 指紋偵測（7秒）═══");
    ui.showMessage("FP Verify", "Place finger");
    Serial.println("請按壓手指進行辨識");
    
    unsigned long startTime = millis();
    const unsigned long timeout = 7000;
    
    while (millis() - startTime < timeout) {
        if (finger.getImage() == FINGERPRINT_OK) {
            
            // 1. 取得「狀態碼」，而不是 ID
            int status = finger.fingerSearch(); 
            
            // 2. 必須嚴格比對狀態碼是否為 FINGERPRINT_OK (0)
            if (status == FINGERPRINT_OK) {
                
                // 3. 狀態確定成功後，才去拿真正的 ID 與分數
                int realId = finger.fingerID;
                int score = finger.confidence;
                
                if (score >= 60) { // 加上信心門檻防護
                    Serial.printf("✅ 指紋辨識成功！ID: %d, 分數: %d\n", realId, score);
                    ui.showMessage("FP OK!", "ID: " + String(realId));
                    successUnlock("Fingerprint #" + String(realId), nullptr);
                    return; // 開門後結束偵測
                } else {
                    Serial.println("⚠️ 匹配成功但分數太低，拒絕開門");
                }
            } else {
                // 如果 status 是 9 (找不到)，就會掉到這裡，不會亂開門了！
                Serial.println("❌ 找不到匹配的指紋");
            }
            
            // 避免連續快速觸發，稍微延遲
            delay(500); 
        }
        delay(50);
    }
    
    Serial.println("❌ 指紋辨識逾時");
    ui.showMessage("FP Timeout", "Try again");
    delay(1500);
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("╔════════════════════════════╗");
    Serial.println("║  智慧門鎖 V2  啟動中...  ║");
    Serial.println("╚════════════════════════════╝");

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Serial.println("✅ I2C 匯流排初始化完成");

    ui.begin();
    ui.showMessage("Booting...", "Init hardware");

    pcf8574_init(PCF_KEYPAD_ADDR);
    pcf8574_init(PCF_STATUS_ADDR);
    Serial.println("✅ PCF8574 初始化完成");

    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);
    Serial.println("✅ 繼電器初始化（門已鎖緊）");

    battery.begin();

    pir.begin();
    Serial.println("✅ 紅外線(PIR) 感測器初始化完成");

    ui.showMessage("Booting...", "Camera init");
    if (!faceSystem.initCamera()) {
        ui.showMessage("ERROR", "Camera failed!");
        delay(3000);
    }

    ui.showMessage("Booting...", "Fingerprint");
    if (!initFingerprint()) {
        Serial.println("⚠️ AS608 指紋模組未找到，功能停用");
    } else {
        Serial.println("✅ AS608 指紋辨識模組初始化成功");
        
        // 🚀 加入這兩行，強制清空 AS608 肚子裡的所有資料！
        finger.emptyDatabase();
        Serial.println("🗑️ [系統維護] 已強制清空 AS608 內部所有幽靈指紋！");
    }

    initAudio();

    ui.showMessage("Booting...", "Connect WiFi");
    bool wifiOK = connectWiFi();
    if (wifiOK) {
        syncTime();
        weatherCache = getWeather();
        lastWeatherUpdate = millis();
    }

    playSound(SOUND_STARTUP);
    setLED(false, false);

    lastActivityTime = millis();

    if (wifiOK) {
        sendTelegramMessage(
            "🔐 智慧門鎖 V2 已啟動（PIR 喚醒模式）\n"
            "IP: " + WiFi.localIP().toString() + "\n"
            "電量: " + battery.toDisplayString() + "\n" +
            getCurrentDateTime()
        );
    }

    ui.showMessage("Ready.", "Waiting nearby...");
    delay(1500);
    ui.display.ssd1306_command(SSD1306_DISPLAYOFF);

    currentState = STATE_SLEEP;
    Serial.println("✅ 啟動完成，進入休眠等待有人靠近...");
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
        if (WiFi.status() == WL_CONNECTED)
            weatherCache = getWeather();
    }

    static unsigned long lastBattCheck = 0;
    if (millis() - lastBattCheck > 30000) {
        lastBattCheck = millis();
        auto b = battery.getStatus(true);
        if (b.lowBattery) {
            static bool alerted = false;
            if (!alerted) {
                alerted = true;
                if (currentState != STATE_SLEEP) playSoundAsync(SOUND_LOW_BATT);
                sendTelegramMessage("⚡ 電量不足：" + String(b.percentage) + "%");
            }
        }
        if (b.percentage < CRITICAL_BATTERY_PCT) {
            setCpuFrequencyMhz(80);
        }
    }

    switch (currentState) {
        case STATE_SLEEP:      handleSleep();     break;
        case STATE_IDLE:       handleIdle();      break;
        case STATE_UNLOCKED:   handleUnlocked();  break;
        case STATE_ALARM:      handleAlarm();     break;
        case STATE_FACE_MGMT:  handleFaceMgmt();  break;
        default:               handleIdle();      break;
    }
}