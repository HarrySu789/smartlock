// src/keypad.h - PCF8574 鍵盤驅動（純 Wire.h 實作）
#pragma once

#include <Wire.h>

// PCF8574 I2C 地址（從 config.h 讀取）
// 需要在包含此檔案前先包含 config.h

// 鍵盤字元對應表（4 row × 4 col）
static const char KEY_MAP[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// 防彈跳設定
#define KEY_DEBOUNCE_MS  30    // 縮短防彈跳延遲（從50ms減至30ms）
#define KEY_HOLD_MS      800   // 長按判斷時間（ms）

static unsigned long lastKeyTime = 0;
static char          lastKey     = 0;

/**
 * 寫入 PCF8574
 * @param addr  I2C 地址
 * @param data  8 bit 資料（1=HIGH, 0=LOW）
 */
static inline void pcf8574_write(uint8_t addr, uint8_t data) {
    Wire.beginTransmission(addr);
    Wire.write(data);
    Wire.endTransmission();
}

/**
 * 從 PCF8574 讀取
 * @param addr  I2C 地址
 * @return      8 bit 資料（1=HIGH, 0=LOW），讀取失敗回傳 0xFF
 */
static inline uint8_t pcf8574_read(uint8_t addr) {
    Wire.requestFrom((uint8_t)addr, (uint8_t)1);
    if (Wire.available()) {
        return Wire.read();
    }
    return 0xFF;  // 讀取失敗，回傳全 HIGH
}

/**
 * 初始化 PCF8574（將所有腳位設為 HIGH）
 * @param addr  I2C 地址
 */
static inline void pcf8574_init(uint8_t addr) {
    pcf8574_write(addr, 0xFF);  // 所有腳位設為 HIGH（輸入模式）
    delayMicroseconds(100);
}

/**
 * 掃描 PCF8574 矩陣鍵盤
 * 回傳：被按下的字元，無按鍵時回傳 0
 *
 * 原理：
 *   P0-P3 為 Row（輸出），P4-P7 為 Col（輸入）
 *   逐一把每個 Row 拉低，然後讀取 Col
 *   若某 Col 為 LOW，代表對應的按鍵被按下
 *
 * 注意：此版本不等待按鍵放開，每次按鍵只返回一次
 *
 * @param addr  PCF8574 的 I2C 地址
 */
char scanKeypad(uint8_t addr) {
    // 防重複觸發：短時間內同一按鍵不回傳
    static unsigned long lastKeyPressTime = 0;
    static char lastReturnedKey = 0;
    
    // 掃描 4 個 Row (P0 ~ P3)
    for (int row = 0; row < 4; row++) {
        // 設定當前 Row 為 LOW (0)，其他 Row 為 HIGH (1)
        // Col (P4 ~ P7) 必須保持 HIGH (1) 以啟動內部上拉電阻作為輸入
        uint8_t outByte = 0xFF ^ (1 << row);
        pcf8574_write(addr, outByte);
        delayMicroseconds(50);  // 等待電位穩定
        
        // 讀取目前的腳位狀態
        uint8_t inByte = pcf8574_read(addr);
        
        // 檢查 4 個 Col (P4 ~ P7 對應 bit 4 ~ 7)
        for (int col = 0; col < 4; col++) {
            // 如果對應的 bit 變成 0 (LOW)，代表該行與該列接通了
            if ((inByte & (1 << (col + 4))) == 0) {
                char key = KEY_MAP[row][col];
                
                // 防重複：如果這個按鍵剛剛返回過，跳過
                unsigned long now = millis();
                if (key == lastReturnedKey && now - lastKeyPressTime < 200) {
                    pcf8574_write(addr, 0xFF);
                    return 0;
                }
                
                // 防彈跳：延遲後再確認一次
                delay(KEY_DEBOUNCE_MS);
                inByte = pcf8574_read(addr);
                if ((inByte & (1 << (col + 4))) != 0) {
                    continue;  // 只是雜訊，繼續掃描
                }
                
                // 記錄這個按鍵，避免短時間內重複返回
                lastReturnedKey = key;
                lastKeyPressTime = now;
                
                // 掃描結束，恢復所有腳位為 HIGH
                pcf8574_write(addr, 0xFF);
                
                lastKey = key;
                lastKeyTime = millis();
                return key;
            }
        }
    }
    
    // 無人按按鍵，恢復所有腳位為 HIGH
    pcf8574_write(addr, 0xFF);
    return 0;
}

/**
 * 等待按鍵（最多 timeoutMs 毫秒）
 * 適合在確認對話框中使用
 * 
 * @param addr        PCF8574 的 I2C 地址
 * @param timeoutMs   逾時時間（毫秒）
 */
char waitForKey(uint8_t addr, unsigned long timeoutMs = 10000) {
    unsigned long start = millis();
    while (millis() - start < timeoutMs) {
        char k = scanKeypad(addr);
        if (k) return k;
        delay(50);
    }
    return 0;  // 逾時
}