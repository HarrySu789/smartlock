// src/pir_sensor.h - HC-SR501 PIR 人體感測器模組
// 門外 PIR：直接 GPIO 輸入 (PIR_OUT_PIN)
// 門內 PIR：透過 PCF8574 #2 (PIR_IN_P) 輸入
#pragma once

#include <Arduino.h>
#include "config.h"
#include "keypad.h"  // for pcf8574_read/write

class PIRSensor {
public:
    PIRSensor() {}

    // ── 初始化 ────────────────────────────────────────
    void begin() {
        // 門外 PIR：直接 GPIO 輸入，啟用下拉電阻（避免浮動訊號）
        pinMode(PIR_OUT_PIN, INPUT_PULLDOWN);
        Serial.printf("✅ 門外 PIR 初始化完成 (GPIO%d, PULLDOWN)\n", PIR_OUT_PIN);

        // 門內 PIR：透過 PCF8574 #2
        // 先將該腳位設為 INPUT 模式，再寫入 HIGH 啟用上拉
        uint8_t status = pcf8574_read(PCF_STATUS_ADDR);
        // 確保 P3 為輸入（寫入 1 設為輸入）
        status |= (1 << PIR_IN_P);
        pcf8574_write(PCF_STATUS_ADDR, status);
        Serial.printf("✅ 門內 PIR 初始化完成 (PCF8574 P%d)\n", PIR_IN_P);
    }

    // ── 讀取門外 PIR 狀態 ────────────────────────────
    // 注意：HC-SR501 未偵測到移動時輸出 HIGH，偵測到時輸出 LOW
    // 回傳：true = 偵測到移動，false = 無移動
    bool isOutsideDetected() {
        return digitalRead(PIR_OUT_PIN) == LOW;
    }

    // ── 讀取門內 PIR 狀態 ────────────────────────────
    // 注意：PCF8574 作為輸入時，必須先寫入 HIGH 才能正確讀取
    // 回傳：true = 偵測到移動，false = 無移動
    bool isInsideDetected() {
        // 確保該腳位為 HIGH（啟用內部上拉）
        uint8_t status = pcf8574_read(PCF_STATUS_ADDR);
        status |= (1 << PIR_IN_P);
        pcf8574_write(PCF_STATUS_ADDR, status);
        delayMicroseconds(50);  // 等待電位穩定

        // 讀取狀態
        status = pcf8574_read(PCF_STATUS_ADDR);
        return (status & (1 << PIR_IN_P)) == 0;  // PIR 輸出為 LOW 表示偵測到
    }

    // ── 讀取兩個 PIR 狀態 ───────────────────────────
    struct Status {
        bool outside;  // 門外
        bool inside;   // 門內
    };

    Status getStatus() {
        Status s;
        s.outside = isOutsideDetected();
        s.inside = isInsideDetected();
        return s;
    }
};