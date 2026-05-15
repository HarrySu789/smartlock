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
OledUI                ui;                      // UI 需要先宣告
FaceRecognitionSystem faceSystem;
FaceDatabase          faceDB;
FaceManager           faceMgr(faceSystem, faceDB, ui);
BatteryMonitor        battery;
PIRSensor             pir;  // PIR 人體感測器

HardwareSerial       fpSerial(1);
Adafruit_Fingerprint finger(&fpSerial);

// ===== 執行時狀態 =====
String       currentPassword = DEFAULT_PASSWORD;
int          failCount       = 0;
SystemState  currentState    = STATE_SLEEP;   // 初始為 SLEEP
String       inputBuffer     = "";
unsigned long unlockTimestamp = 0;
String       lastUnlockName  = "";

// 天氣資訊快取
WeatherInfo  weatherCache;
unsigned long lastWeatherUpdate = 0;

// ===== 時間戳記 =====
unsigned long lastPIRCheck     = 0;  // PIR 檢查
unsigned long lastActivityTime = 0;  // 最後活動時間
unsigned long lastWeatherAnnounceTime = 0;  // 上次天氣播報時間（防連觸）

// ===== Telegram 待登錄 =====
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

// ===== 更新「最後活動時間」====
void markActivity() {
    lastActivityTime = millis();
}

// ===== LED 控制 =====
void setLED(bool green, bool red) {
    uint8_t status = pcf8574_read(PCF_STATUS_ADDR);
    
    // 清除 LED 腳位（設為 0 = LOW = 點亮）
    status &= ~(1 << LED_GREEN_P);
    status &= ~(1 << LED_RED_P);
    
    // 設定 LED 狀態（0 = LOW = 點亮，1 = HIGH = 熄滅）
    if (!green) status |= (1 << LED_GREEN_P);  // green=false → HIGH → 熄滅
    if (!red)   status |= (1 << LED_RED_P);    // red=false → HIGH → 熄滅
    
    pcf8574_write(PCF_STATUS_ADDR, status);
}

// ===== 解鎖成功 =====
void successUnlock(const String& name, camera_fb_t* photoFb) {
    Serial.printf("🔓 解鎖！[%s]\n", name.c_str());
    failCount = 0;
    lastUnlockName = name;
    currentState = STATE_UNLOCKED;
    unlockTimestamp = millis();
    markActivity();

    // 開鎖
    unlockDoor(UNLOCK_DURATION_MS);
    playSoundAsync(SOUND_UNLOCK);
    setLED(true, false);

    // 取得天氣提示
    String weatherMsg = getWeatherMessage(weatherCache);

    // OLED 顯示
    String weatherDisplay = WEATHER_NOTIFY_EN ? weatherMsg : "";
    ui.showUnlocked(name, weatherDisplay);

    // Telegram 通知
    sendTelegramMessage("✅ 解鎖：" + name + "\n" + getCurrentDateTime() + "\n" + weatherMsg);
}

// ===== 失敗處理 =====
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

// ===== 鍵盤輸入處理 =====
void handleKeyInput(char key) {
    markActivity();
    playSoundAsync(SOUND_BEEP);
    
    // 序列埠輸出按鍵（顯示目前按下的按鍵）
    Serial.printf("══════════════════════════\n");
    Serial.printf("🔢 按下按鍵: '%c' (ASCII: %d)\n", key, (int)key);
    Serial.printf("══════════════════════════\n");

    // A 鍵（長按進入管理模式，需先輸入管理密碼）
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
        // 確認密碼
        Serial.printf("  → 按下 # 鍵（確認）\n");
        Serial.printf("  → 輸入緩衝: \"%s\" vs 密碼: \"%s\"\n", 
                      inputBuffer.c_str(), currentPassword.c_str());
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
        // D 鍵：刪除最後一位
        if (inputBuffer.length() > 0) {
            inputBuffer.remove(inputBuffer.length() - 1);
            ui.showPasswordInput(inputBuffer.length());
            Serial.printf("  → 刪除一位，目前輸入: \"%s\"\n", inputBuffer.c_str());
        }
    } else if (isDigit(key)) {
        inputBuffer += key;
        ui.showPasswordInput(inputBuffer.length());
        Serial.printf("  → 已輸入 %d 位: \"%s\"\n", 
                      inputBuffer.length(), inputBuffer.c_str());
    }
}

// ===== STATE_SLEEP：休眠，等待有人靠近 =====
void handleSleep() {
    // 用低頻率量測，節省功耗
    if (millis() - lastPIRCheck < 500) return;
    lastPIRCheck = millis();

    // 檢查門外 PIR
    if (pir.isOutsideDetected()) {
        Serial.println("👤 門外偵測到人體，喚醒系統");

        // ── 喚醒序列 ──
        // 1. 開啟 OLED
        ui.showMessage("Someone outside!", "Wake up system");

        // 2. 播放喚醒提示音
        playSoundAsync(SOUND_BEEP);

        // 3. 切換狀態
        currentState = STATE_IDLE;
        lastActivityTime = millis();

        Serial.println("✅ 系統已喚醒");
        return;
    }

    // 檢查門內 PIR（休眠時也要檢查天氣播報）
    if (pir.isInsideDetected()) {
        // 檢查冷卻時間
        unsigned long now = millis();
        if (now - lastWeatherAnnounceTime > (PIR_COOLDOWN_SEC * 1000UL)) {
            lastWeatherAnnounceTime = now;
            Serial.println("🏠 門內偵測到人體，播報天氣");

            // 播報天氣
            if (WEATHER_NOTIFY_EN && weatherCache.valid) {
                String weatherMsg = getWeatherMessage(weatherCache);
                ui.showMessage("Weather:", weatherMsg);
            }
        }
    }
}

// ===== STATE_IDLE：正常待機，全功能運作 =====
void handleIdle() {
    // ── PIR 檢查與自動進入休眠 ──────────────────────────
    if (millis() - lastPIRCheck >= 500) {
        lastPIRCheck = millis();
        
        // 檢查門外 PIR
        bool outsideDetected = pir.isOutsideDetected();
        
        // 檢查門內 PIR（天氣播報）
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

        // 如果門外沒偵測到人，開始計時進入休眠
        if (!outsideDetected) {
            unsigned long idleSec = (millis() - lastActivityTime) / 1000;
            if (idleSec >= SLEEP_TIMEOUT_SEC) {
                Serial.printf("💤 %lu 秒無人就入休眠\n", (unsigned long)SLEEP_TIMEOUT_SEC);

                // 清理畫面後關閉 OLED
                ui.showMessage("Standby...", "Entering sleep");
                delay(800);
                ui.display.ssd1306_command(SSD1306_DISPLAYOFF);

                setLED(false, false);
                inputBuffer = "";
                currentState = STATE_SLEEP;
                return;
            }
        } else {
            // 門外有人，更新活動時間
            markActivity();
        }
    }

    // ── OLED 待機畫面（每秒更新）────────────────────
    static unsigned long lastDisplayUpdate = 0;
    if (millis() - lastDisplayUpdate > 1000) {
        lastDisplayUpdate = millis();

        // 確保 OLED 是開啟狀態
        ui.display.ssd1306_command(SSD1306_DISPLAYON);

        auto batt = battery.getStatus();
        struct tm t;
        getLocalTime(&t);
        char timeStr[6];
        strftime(timeStr, 6, "%H:%M", &t);
        ui.showIdle(String(timeStr), weatherCache.temp,
                    weatherCache.rainToday, batt.percentage, batt.charging);
    }

    // ── 鍵盤 ────────────────────────────────────────
    char key = scanKeypad(PCF_KEYPAD_ADDR);
    if (key) handleKeyInput(key);

    // ── 指紋輪詢 ─────────────────────────────────────
    if (FINGERPRINT_EN) {
        if (finger.getImage() == FINGERPRINT_OK) {
            markActivity();
            int fpId = verifyFingerprint();
            if (fpId >= 0) {
                successUnlock("Fingerprint #" + String(fpId), nullptr);
            } else if (fpId == -1) {
                failedAttempt();
            }
        }
    }

    // ── 人臉辨識 ──────────────────────────────
    static unsigned long lastFaceCheck = 0;
    if (millis() - lastFaceCheck > FACE_SCAN_INTERVAL_MS) {
        lastFaceCheck = millis();
        handleFaceCheck();
    }
}

// ===== 人臉辨識處理 =====
void handleFaceCheck() {
    camera_fb_t* fb = faceSystem.capture();
    if (!fb) return;

    // 檢查是否正在進行人臉登錄（Telegram 觸發）
    if (pendingEnroll) {
        checkPendingEnrollWrapper(fb);
        faceSystem.returnFrame(fb);
        return;
    }

    ui.showVerifying("Face Recognition");

    auto result = faceSystem.process(fb);

    if (result.recognized) {
        // 已知人員 → 解鎖
        markActivity();
        successUnlock(result.name, fb);
    } else if (result.face_detected) {
        // 陌生人偵測
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

// ===== 待登錄處理包裝函式 =====
void checkPendingEnrollWrapper(camera_fb_t* fb) {
    if (!pendingEnroll || !fb) return;

    static unsigned long pendingStart = 0;
    static int enrollCount = 0;

    if (pendingStart == 0) {
        pendingStart = millis();
        enrollCount = 0;
        Serial.printf("[TG 登錄] 開始：%s\n", pendingEnrollName.c_str());
    }

    // 30 秒逾時
    if (millis() - pendingStart > 30000) {
        pendingEnroll = false;
        pendingStart = 0;
        enrollCount = 0;
        sendTelegramMessage("⏰ 登錄逾時：" + pendingEnrollName + "\n請重新使用 /face_enroll 指令");
        return;
    }

    // 嘗試登錄
    bool ok = faceSystem.enroll(fb, pendingEnrollName);
    if (ok) {
        enrollCount++;
        Serial.printf("[TG 登錄] %s 第 %d/%d 張\n", pendingEnrollName.c_str(), enrollCount, FACE_ENROLL_SAMPLES);

        if (enrollCount >= FACE_ENROLL_SAMPLES) {
            pendingEnroll = false;
            pendingStart = 0;
            enrollCount = 0;
            sendTelegramMessage("✅ 人臉登錄完成：" + pendingEnrollName + "\n已儲存 " + String(FACE_ENROLL_SAMPLES) + " 張樣本");
            Serial.printf("[TG 登錄] 完成：%s\n", pendingEnrollName.c_str());
        }
    }
}

// ===== STATE_UNLOCKED：已解鎖倒數 =====
void handleUnlocked() {
    // 只使用 relay.h 中的 updateRelay() 來自動鎖門，避免兩個計時機制衝突
    updateRelay();

    // 顯示剩餘秒數（從 relay.h 取得）
    int remaining = getUnlockRemainingSeconds();
    if (remaining < 0) remaining = 0;

    // 每秒更新顯示
    static int lastRemaining = -1;
    if (remaining != lastRemaining) {
        lastRemaining = remaining;
        String msg = "Auto-lock in " + String(remaining) + "s";
        String weather = WEATHER_NOTIFY_EN ? getWeatherMessage(weatherCache) : "";
        ui.showUnlocked(lastUnlockName, weather.length() > 0 ? weather : msg);
    }

    // 檢查是否已自動鎖門
    if (!isDoorUnlocked()) {
        setLED(false, false);
        currentState = STATE_IDLE;
        lastRemaining = -1;
        Serial.println("🔒 已自動鎖門");
    }
}

// ===== STATE_ALARM：警報 =====
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

// ===== STATE_FACE_MGMT：人臉管理選單 =====
void handleFaceMgmt() {
    ui.showFaceMenu(faceSystem.getCount());
    markActivity();

    char key = waitForKey(PCF_KEYPAD_ADDR, 15000);
    markActivity();

    if (key == '1') {
        // 人臉登錄：透過 Telegram 操作
        ui.showMessage("Add Face", "Use Telegram:\n/face_enroll [name]");
        sendTelegramMessage("📷 請使用 Telegram 登錄：\n/face_enroll 姓名");
        delay(3000);
    } else if (key == '2') {
        // 指紋登錄：顯示操作說明
        ui.showMessage("FP Enroll", "Press finger\nthen lift");
        sendTelegramMessage("👆 指紋登錄說明：\n請在指紋感測器上按壓手指\n聽到嗶聲後提起，再重新按壓\n重複 8-12 次直到成功");
        Serial.println("═══ 指紋登錄說明 ═══");
        Serial.println("請在指紋感測器上按壓手指");
        Serial.println("聽到嗶聲後提起，再重新按壓");
        Serial.println("重複 8-12 次直到成功");
        Serial.println("═══════════════════");
        delay(4000);
    } else if (key == '3') {
        auto list = faceSystem.getList();
        if (list.empty()) ui.showMessage("No faces", "Database empty");
        else              ui.showList("Enrolled Faces", list);
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

// ===== setup =====
void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("╔════════════════════════════╗");
    Serial.println("║  智慧門鎖 V2  啟動中...  ║");
    Serial.println("╚══════════════════════════════╝");

    // 1. I2C 初始化
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Serial.println("✅ I2C 匯流排初始化完成");

    // 2. OLED
    ui.begin();
    ui.showMessage("Booting...", "Init hardware");

    // 3. PCF8574 初始化
    pcf8574_init(PCF_KEYPAD_ADDR);
    pcf8574_init(PCF_STATUS_ADDR);
    Serial.println("✅ PCF8574 初始化完成");

    // 4. 繼電器
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);  // 確保鎖緊
    Serial.println("✅ 繼電器初始化（門已鎖緊）");

    // 5. 電池監控
    battery.begin();

    // 6. PIR 人體感測器
    pir.begin();
    Serial.println("✅ 紅外線(PIR) 感測器初始化完成");

    // 7. 鏡頭
    ui.showMessage("Booting...", "Camera init");
    if (!faceSystem.initCamera()) {
        ui.showMessage("ERROR", "Camera failed!");
        delay(3000);
    }

    // 8. SPIFFS + 人臉資料庫
    ui.showMessage("Booting...", "Load face DB");
    faceDB.begin();
    faceMgr.restoreDatabase();

    // 9. 指紋
    ui.showMessage("Booting...", "Fingerprint");
    fpSerial.begin(AS608_BAUD, SERIAL_8N1, AS608_RX_PIN, AS608_TX_PIN);
    if (!finger.verifyPassword()) {
        Serial.println("⚠️ AS608 指紋模組未找到，功能停用");
    } else {
        Serial.println("✅ AS608 指紋辨識模組初始化成功");
    }

    // 10. 音頻
    initAudio();

    // 11. WiFi + NTP
    ui.showMessage("Booting...", "Connect WiFi");
    bool wifiOK = connectWiFi();
    if (wifiOK) {
        syncTime();
        weatherCache = getWeather();
        lastWeatherUpdate = millis();
    }

    // 12. 啟動音效
    playSound(SOUND_STARTUP);
    setLED(false, false);

    // 13. 初始化活動時間
    lastActivityTime = millis();

    // 14. 啟動完成 → 進入 SLEEP 等待有人靠近
    if (wifiOK) {
        sendTelegramMessage(
            "🔐 智慧門鎖 V2 已啟動（PIR 喚醒模式）\n"
            "IP: " + WiFi.localIP().toString() + "\n"
            "電量: " + battery.toDisplayString() + "\n" +
            getCurrentDateTime()
        );
    }

    // OLED 顯示休眠訊息後關閉螢幕
    ui.showMessage("Ready.", "Waiting nearby...");
    delay(1500);
    ui.display.ssd1306_command(SSD1306_DISPLAYOFF);

    currentState = STATE_SLEEP;

    Serial.println("✅ 啟動完成，進入休眠等待有人靠近...");
}

// ===== loop =====
void loop() {
    // ── 週期性任務（不受狀態影響）────────────────────

    // WiFi 維持
    maintainWiFi();

    // Telegram 輪詢（3 秒，SLEEP 狀態下仍執行）
    static unsigned long lastBotPoll = 0;
    if (millis() - lastBotPoll > BOT_POLL_MS) {
        lastBotPoll = millis();
        handleTelegramCommands();
        
        // 若 Telegram 下達 /unlock，強制喚醒
        if (currentState == STATE_SLEEP && isDoorUnlocked()) {
            ui.display.ssd1306_command(SSD1306_DISPLAYON);
            currentState = STATE_UNLOCKED;
            markActivity();
        }
    }

    // 天氣更新（10 分鐘）
    if (millis() - lastWeatherUpdate > WEATHER_UPDATE_MS) {
        lastWeatherUpdate = millis();
        if (WiFi.status() == WL_CONNECTED)
            weatherCache = getWeather();
    }

    // 電池監控（30 秒）
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
            setCpuFrequencyMhz(80);  // 省電模式
        }
    }

    // ── 狀態機 ────────────────────────────────────────
    switch (currentState) {
        case STATE_SLEEP:      handleSleep();     break;
        case STATE_IDLE:       handleIdle();      break;
        case STATE_UNLOCKED:   handleUnlocked();  break;
        case STATE_ALARM:      handleAlarm();     break;
        case STATE_FACE_MGMT:  handleFaceMgmt();  break;
        default:               handleIdle();      break;
    }
}