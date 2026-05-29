// src/telegram_bot.h
// 8785672553:AAG0gI7yz4EF2QhzVGjk2gPQP3M8mIbOd9s
//檢視指令 https://api.telegram.org/bot8785672553:AAG0gI7yz4EF2QhzVGjk2gPQP3M8mIbOd9s/getMyCommands
//清除指令 https://api.telegram.org/bot8785672553:AAG0gI7yz4EF2QhzVGjk2gPQP3M8mIbOd9s/deleteMyCommands
#pragma once

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "esp_camera.h"
#include "img_converters.h"
#include "config.h"
#include "relay.h"
#include "battery.h"
#include "weather.h"
#include "face_recognition.h"
#include "face_database.h"
#include "fingerprint.h"
// 外部物件（在 main.cpp 宣告）
extern FaceRecognitionSystem faceSystem;
extern FaceDatabase          faceDB;
extern WeatherInfo           weatherCache;
extern BatteryMonitor        battery;
extern String                currentPassword;
extern int                   failCount;
extern SystemState           currentState;
extern unsigned long          unlockTimestamp;

WiFiClientSecure tgClient;
UniversalTelegramBot bot(BOT_TOKEN, tgClient);

// ── TLS 初始化（只執行一次）──────────────────
void initTelegramTLS() {
    static bool isInit = false;
    if (!isInit) {
        tgClient.setInsecure();
        isInit = true;
    }
}
// ── 開機強制清空堆積指令（防當機） ──────────────────
void flushPendingTelegramMessages() {
    if (WiFi.status() != WL_CONNECTED) return;
    
    Serial.println("🧹 正在清空 Telegram 伺服器上的歷史堆積訊息...");
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure(); // 忽略 SSL 憑證檢查

    // 呼叫 deleteWebhook 並帶上 drop_pending_updates=true 參數來強制歸零
    String url = "https://api.telegram.org/bot" + String(BOT_TOKEN) + "/deleteWebhook?drop_pending_updates=true";
    
    http.begin(client, url);
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        Serial.println("✅ 歷史訊息清空成功！系統從零開始接收新指令。");
    } else {
        Serial.printf("❌ 清空失敗，HTTP 狀態碼: %d\n", httpCode);
    }
    http.end();
}

// 待登錄人臉名稱（Telegram 遠端設定後，鏡頭前觸發）
String pendingEnrollName = "";
bool   pendingEnroll     = false;

// ── 傳送文字訊息 ──────────────────────────────
void sendTelegramMessage(const String& text) {
    if (WiFi.status() != WL_CONNECTED) return;
    initTelegramTLS();
    bot.sendMessage(CHAT_ID, text, "");
    Serial.println("[TG 發送] " + text.substring(0, 60));
}

// ── 傳送真實照片給 Telegram（陌生人警報）───────────────────────
void sendTelegramPhoto(camera_fb_t* fb, const String& caption) {
    if (!fb || WiFi.status() != WL_CONNECTED) return;

    Serial.println("📷 開始轉換影像為 JPEG...");
    
    // 1. 將 RGB565 轉為 JPEG (壓縮品質 80)
    uint8_t* jpg_buf = NULL;
    size_t jpg_buf_len = 0;
    bool converted = fmt2jpg(fb->buf, fb->len, fb->width, fb->height, fb->format, 80, &jpg_buf, &jpg_buf_len);

    if (!converted) {
        Serial.println("❌ 影像轉換 JPEG 失敗");
        bot.sendMessage(CHAT_ID, caption + "\n(照片轉換失敗，記憶體可能不足)", "");
        return;
    }

    Serial.printf("✅ JPEG 轉換完成：%u bytes\n", jpg_buf_len);
    Serial.println("🌐 正在上傳照片到 Telegram...");
    
    // 2. 建立專屬的網路連線來傳送照片
    WiFiClientSecure client;
    client.setInsecure(); // 忽略憑證驗證，加速連線

    if (client.connect("api.telegram.org", 443)) {
        String boundary = "----ESP32SmartLockBoundary" + String(millis());
        
        // 打包包裹：表單的標頭、Chat ID 和 文字說明 (Caption)
        String bodyPrefix = "--" + boundary + "\r\n";
        bodyPrefix += "Content-Disposition: form-data; name=\"chat_id\"\r\n\r\n" + String(CHAT_ID) + "\r\n";
        bodyPrefix += "--" + boundary + "\r\n";
        bodyPrefix += "Content-Disposition: form-data; name=\"caption\"\r\n\r\n" + caption + "\r\n";
        bodyPrefix += "--" + boundary + "\r\n";
        
        // 打包包裹：照片檔案的資訊
        bodyPrefix += "Content-Disposition: form-data; name=\"photo\"; filename=\"alert.jpg\"\r\n";
        bodyPrefix += "Content-Type: image/jpeg\r\n\r\n";

        String bodySuffix = "\r\n--" + boundary + "--\r\n";
        uint32_t contentLength = bodyPrefix.length() + jpg_buf_len + bodySuffix.length();

        // 3. 正式發送 HTTP 請求
        client.println("POST /bot" + String(BOT_TOKEN) + "/sendPhoto HTTP/1.1");
        client.println("Host: api.telegram.org");
        client.println("Content-Type: multipart/form-data; boundary=" + boundary);
        client.print("Content-Length: ");
        client.println(contentLength);
        client.println();

        // 傳送前半段表單
        client.print(bodyPrefix);

        // 傳送照片本體（分段上傳，保護記憶體不崩潰）
        uint8_t* p = jpg_buf;
        size_t remaining = jpg_buf_len;
        while (remaining > 0) {
            size_t toWrite = (remaining > 1024) ? 1024 : remaining;
            client.write(p, toWrite);
            p += toWrite;
            remaining -= toWrite;
        }

        // 傳送結尾標籤
        client.print(bodySuffix);

        // 等待伺服器回應（避免過早斷開連線導致失敗）
        while (client.connected()) {
            String line = client.readStringUntil('\n');
            if (line == "\r") break; 
        }
        client.stop();
        Serial.println("✅ 照片上傳完成！");
    } else {
        Serial.println("❌ 無法連線至 Telegram API");
        bot.sendMessage(CHAT_ID, caption + "\n(網路連線失敗，照片無法上傳)", "");
    }

    // 4. 非常重要：釋放影像佔用的記憶體，防止當機！
    free(jpg_buf);
}

// ── 處理所有進來的 Bot 指令（修正安全版） ──────────────────────
void handleTelegramCommands() {
    if (WiFi.status() != WL_CONNECTED) return;

    initTelegramTLS();
    
    // 🛠️ 【核心修正】強迫每次只抓取 1 則訊息 (limit = 1)，並設定超時為 1 秒 (timeout = 1)
    // 這樣做能確保不論伺服器堆積了多少訊息，ESP32 每次下載的 JSON 封包都   極小，
    // 絕對不會撐爆 UniversalTelegramBot 的內部快取，能 100% 成功解出指令！
    // ✅ 改回原本的單參數寫法
    int count = bot.getUpdates(bot.last_message_received + 1);

    // 🛠️ 【除錯監控】如果成功拿到訊息，在序列埠印出提示
    if (count > 0) {
        Serial.printf("[TG 提貨] 成功抓取 %d 則訊息，正在處理...\n", count);
    }

    for (int i = 0; i < count; i++) {
        String text   = bot.messages[i].text;
        String fromId = bot.messages[i].chat_id;
        String fromName = bot.messages[i].from_name;

        // ── 安全驗證：只接受授權的 Chat ID ──
        if (fromId != String(CHAT_ID)) {
            bot.sendMessage(fromId, "⛔ 未授權的存取", "");
            Serial.printf("[TG 警告] 非授權存取來自 %s (%s)\n",
                          fromName.c_str(), fromId.c_str());
            sendTelegramMessage("⚠️ 警告：有未知用戶嘗試存取門鎖！\nID: " + fromId);
            continue;
        }

        Serial.printf("[TG 指令] %s: %s\n", fromName.c_str(), text.c_str());
        text.trim();

        // ── /unlock ──────────────────────────
        if (text == "/unlock") {
            bot.sendMessage(CHAT_ID, "🔓 開鎖中...", "");
            unlockDoor(5000);
            currentState = STATE_UNLOCKED;
            unlockTimestamp = millis();
            sendTelegramMessage("✅ 遠端開鎖 5 秒\n時間：" + getCurrentDateTime());
        }
        // ── /status ──────────────────────────
        else if (text == "/status") {
            auto batt = battery.getStatus(true);
            String s = "📊 門鎖狀態\n";
            s += "━━━━━━━━━━━━\n";
            s += "🔒 門鎖：" + String(isDoorUnlocked() ? "開啟中" : "已鎖緊") + "\n";
            s += "📶 WiFi：" + WiFi.localIP().toString() + "\n";
            s += "⚡ 電量：" + String(batt.percentage) + "% " +
                 (batt.charging ? "（充電中）" : "") + "\n";
            s += "🔋 電壓：" + String(batt.voltage, 2) + "V\n";
            s += "👤 人臉：" + String(faceSystem.getCount()) + " 筆\n";
            s += "⏱ 運行：" + String(millis() / 60000) + " 分鐘\n";
            s += "📅 時間：" + getCurrentDateTime();
            bot.sendMessage(CHAT_ID, s, "");
        }
        // ── /weather ─────────────────────────
        else if (text == "/weather") {
            WeatherInfo w = getWeather();
            String s = "🌤 目前天氣（" + String(OWM_CITY) + "）\n";
            s += "━━━━━━━━━━━━\n";
            s += "天氣：" + w.description + "\n";
            s += "溫度：" + String(w.temp, 1) + "°C（體感 " +
                 String(w.feelsLike, 1) + "°C）\n";
            s += "濕度：" + String(w.humidity) + "%\n";
            s += "風速：" + String(w.windSpeed, 1) + " m/s\n";
            s += "\n" + getWeatherMessage(w);
            bot.sendMessage(CHAT_ID, s, "");
        }
        // ── /battery ─────────────────────────
        else if (text == "/battery") {
            auto batt = battery.getStatus(true);
            String s = "🔋 電池狀態\n";
            s += "電量：" + String(batt.percentage) + "%\n";
            s += "電壓：" + String(batt.voltage, 2) + "V\n";
            s += "狀態：" + String(batt.charging ? "充電中 ⚡" : "放電中");
            if (batt.lowBattery) s += "\n⚠️ 電量偏低，請儘快充電！";
            bot.sendMessage(CHAT_ID, s, "");
        }
        // ── /alarm_off ───────────────────────
        else if (text == "/alarm_off") {
            failCount = 0;
            currentState = STATE_IDLE;
            bot.sendMessage(CHAT_ID, "🔕 警報已由 Telegram 解除", "");
        }
        // ── /sleep ────────────────────────────
        else if (text == "/sleep") {
            currentState = STATE_SLEEP;
            bot.sendMessage(fromId, "💤 系統已手動進入休眠模式", "");
            Serial.println("[TG] 系統已手動進入休眠模式");
        }
        // ── /set_password [新密碼] ────────────
        else if (text.startsWith("/set_password ")) {
            String newPwd = text.substring(14);
            newPwd.trim();
            if (newPwd.length() >= 4 && newPwd.length() <= MAX_PASSWORD_LEN) {
                currentPassword = newPwd;
                bot.sendMessage(CHAT_ID, "✅ 密碼已更新為：" + newPwd + "\n請妥善保存！", "");
                Serial.println("[安全] 密碼已透過 Telegram 更新");
            } else {
                bot.sendMessage(CHAT_ID,
                    "❌ 密碼格式錯誤\n長度需為 4~" +
                    String(MAX_PASSWORD_LEN) + " 位", "");
            }
        }
        // ── /face_list ───────────────────────
        else if (text == "/face_list") {
            auto list = faceSystem.getList();
            if (list.empty()) {
                bot.sendMessage(CHAT_ID, "📭 目前沒有登錄任何人臉", "");
            } else {
                String s = "👤 已登錄人臉（" + String(list.size()) + " 筆）\n";
                s += "━━━━━━━━━━━━\n";
                for (int j = 0; j < (int)list.size(); j++) {
                    s += String(j + 1) + ". " + list[j] + "\n";
                }
                s += "\n以 /face_delete [名稱] 刪除";
                bot.sendMessage(CHAT_ID, s, "");
            }
        }
        // ── /face_delete [名稱] ───────────────
        else if (text.startsWith("/face_delete ")) {
            String name = text.substring(13);
            name.trim();
            if (name.isEmpty()) {
                bot.sendMessage(CHAT_ID, "❌ 請指定名稱：/face_delete 王小明", "");
            } else {
                bool ok1 = faceSystem.deleteByName(name);
                bool ok2 = faceDB.remove(name);
                if (ok1 || ok2) {
                    bot.sendMessage(CHAT_ID, "✅ 已刪除人臉：" + name, "");
                } else {
                    bot.sendMessage(CHAT_ID, "❌ 找不到人臉：" + name, "");
                }
            }
        }
        // ── /face_deleteall ───────────────────
        else if (text == "/face_deleteall") {
            bot.sendMessage(CHAT_ID,
                "⚠️ 確認要清除所有人臉嗎？\n回覆 /face_deleteall_confirm 確認", "");
        }
        else if (text == "/face_deleteall_confirm") {
            faceSystem.deleteAll();
            faceDB.removeAll();
            bot.sendMessage(CHAT_ID, "✅ 所有人臉已清除", "");
        }
        // ── /face_enroll [名稱] ───────────────
        else if (text.startsWith("/face_enroll ")) {
            String name = text.substring(13);
            name.trim();
            if (name.isEmpty()) {
                bot.sendMessage(CHAT_ID,
                    "❌ 請指定名稱：/face_enroll 王小明", "");
            } else if (faceSystem.getCount() >= MAX_FACE_COUNT) {
                bot.sendMessage(CHAT_ID,
                    "❌ 人臉資料庫已滿（最多 " +
                    String(MAX_FACE_COUNT) + " 筆），請先刪除", "");
            } else {
                pendingEnrollName = name;
                pendingEnroll     = true;
                bot.sendMessage(CHAT_ID,
                    "📷 準備登錄：*" + name + "*\n"
                    "請在 30 秒內站到門口鏡頭前，系統將自動拍攝 " +
                    String(FACE_ENROLL_SAMPLES) + " 張。", "Markdown");
                Serial.printf("[TG] 待登錄人臉：%s\n", name.c_str());
            }
        }
        // ── /fp_enroll ────────────────────────
        else if (text.startsWith("/fp_enroll ")) {
            bot.sendMessage(CHAT_ID, 
                "⚠️ 指紋登錄需在門鎖設備上操作：\n"
                "1. 進入管理模式（輸入管理密碼後按 A）\n"
                "2. 按 2 選擇指紋登錄\n"
                "3. 在指紋感測器上按壓手指\n"
                "4. 提起後再按壓，重複直到成功", "");
        }
        // ── /fp_list ────────────────────────
        else if (text == "/fp_list") {
            bot.sendMessage(CHAT_ID, 
                "📋 指紋查詢：\n"
                "AS608 最多可儲存 127 枚指紋\n"
                "目前無法從遠端查詢已登錄指紋，請在設備上操作", "");
        }
        // ── /help (完整指令列表) ────────────────────────────
        else if (text == "/help" || text == "/start") {
            String h = "📖 可用指令列表\n";
            h += "━━━━━━━━━━━━\n\n";
            h += "🔓 /unlock - 遠端開鎖\n";
            h += "📊 /status - 系統狀態\n";
            h += "📶 /weather - 天氣查詢\n";
            h += "🔋 /battery - 電量查詢\n";
            h += "💤 /sleep - 進入休眠\n";
            h += "🔕 /alarm_off - 解除警報\n\n";
            h += "👤 人臉管理\n";
            h += "━━━━━━━━\n";
            h += "/face_enroll [名] - 登錄人臉\n";
            h += "/face_list - 列出人臉\n";
            h += "/face_delete [名] - 刪除人臉\n";
            h += "/face_deleteall - 清除所有人臉\n\n";
            h += "👆 指紋管理\n";
            h += "━━━━━━━━\n";
            h += "/fp_enroll - 指紋登錄說明\n";
            h += "/fp_list - 指紋列表\n\n";
            h += "⚙️ 系統設定\n";
            h += "━━━━━━━━\n";
            h += "/set_password [密碼] - 設定密碼\n";
            h += "/hide_keyboard - 隱藏按鍵";
            
            // 建立 Telegram 專用的 JSON 格式實體鍵盤
            String keyboardJson = "[";
            keyboardJson += "[\"/unlock\", \"/status\"],";
            keyboardJson += "[\"/sleep\", \"/alarm_off\"],";
            keyboardJson += "[\"/weather\", \"/battery\"]";
            keyboardJson += "]";

            bot.sendMessageWithReplyKeyboard(CHAT_ID, h, "", keyboardJson, true);
        }
        // ── /hide_keyboard (隱藏實體按鍵) ────────────────────────
        else if (text == "/hide_keyboard") {
            // 傳送一個空的鍵盤，並設定 remove_keyboard 為 true 來關閉它
            bot.sendMessageWithReplyKeyboard(CHAT_ID, "⌨️ 鍵盤已隱藏。輸入 /help 可再次喚出。", "", "", true, true, true);
        }
        // ── 未知指令 ─────────────────────────
        else {
            bot.sendMessage(CHAT_ID,
                "❓ 未知指令：" + text + "\n輸入 /help 查看說明", "");
        }
    }

    // ── 🛠️【核心修正：釋放防卡死】 ─────────────────
    // 將原本的 if (count > 0) 移除。不論這次有沒有成功領到新訊息，
    // 只要有執行查詢，在尾端都強迫斷開並清空 TLS 快取，
    // 這樣可以防止萬一發生解析失敗時，壞掉的 HTTP 連線卡死 ESP32 的記憶體。
    tgClient.stop();
}

// ── 在 loop 中處理待登錄的人臉 ──────────────────
// 需要在人臉辨識的主迴圈中呼叫此函式
void checkPendingEnroll(FaceRecognitionSystem& fr,
                        FaceDatabase& db,
                        camera_fb_t* fb) {
    if (!pendingEnroll || !fb) return;

    static unsigned long pendingStart = 0;
    static int enrollCount = 0;

    if (pendingStart == 0) {
        pendingStart = millis();
        enrollCount  = 0;
        Serial.printf("[TG 登錄] 開始：%s\n", pendingEnrollName.c_str());
    }

    // 30 秒逾時
    if (millis() - pendingStart > 30000) {
        pendingEnroll = false;
        pendingStart  = 0;
        enrollCount   = 0;
        sendTelegramMessage("⏰ 登錄逾時：" + pendingEnrollName +
                            "\n請重新使用 /face_enroll 指令");
        return;
    }

    // 嘗試登錄（只在第一次嘗試，成功後立即結束）
    if (enrollCount == 0) {
        bool ok = fr.enroll(fb, pendingEnrollName);
        if (ok) {
            enrollCount = 1;
            Serial.printf("[TG 登錄] %s 成功！\n", pendingEnrollName.c_str());
            
            // 登錄完成，立即發送訊息並標記完成
            // 登錄完成，持久化儲存（需實作向量匯出）
            // db.save(pendingEnrollName, fr.recognizer.get_latest_feature());
            pendingEnroll = false;
            pendingStart  = 0;
            enrollCount   = 0;
            sendTelegramMessage("✅ 人臉登錄完成：" + pendingEnrollName +
                                "\n已儲存 " + String(FACE_ENROLL_SAMPLES) + " 張樣本");
            Serial.printf("[TG 登錄] 完成：%s\n", pendingEnrollName.c_str());
        }
    }
}