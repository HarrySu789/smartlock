// src/ultrasonic.h
// HC-SR04 超聲波近接偵測模組
// TRIG：PCF8574 #2 P3（I2C 輸出）
// ECHO：GPIO9 (D10)（直接 GPIO，經 1kΩ/2kΩ 分壓）
#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "keypad.h"  // 獲取 pcf8574_write/read 函式

class UltrasonicSensor {
public:
    UltrasonicSensor() {}

    // ── 初始化 ────────────────────────────────────────
    void begin() {
        pinMode(US_ECHO_PIN, INPUT);
        // TRIG 預設 LOW（透過 PCF8574）
        uint8_t state = pcf8574_read(PCF_STATUS_ADDR);
        state &= ~(1 << US_TRIG_P);
        pcf8574_write(PCF_STATUS_ADDR, state);
        Serial.println("✅ HC-SR04 初始化完成");
        Serial.printf("   TRIG = PCF8574 #2 P%d\n", US_TRIG_P);
        Serial.printf("   ECHO = GPIO%d (D10)\n", US_ECHO_PIN);
    }

    // ── 量測距離（公分）────────────────────────────────
    // 回傳值：距離 cm（2~400cm 有效）
    //         -1 = 超時（無回波，表示距離太遠或無障礙物）
    float measureCm() {
        // 讀取目前狀態，設定 TRIG 腳位
        uint8_t currentState = pcf8574_read(PCF_STATUS_ADDR);
        
        // 發送觸發脈衝（透過 PCF8574 I2C）
        // PCF8574 I2C write 約 150μs，兩次合計約 300μs
        // HC-SR04 只需 ≥10μs，300μs 完全符合
        pcf8574_write(PCF_STATUS_ADDR, currentState | (1 << US_TRIG_P));   // TRIG HIGH
        pcf8574_write(PCF_STATUS_ADDR, currentState & ~(1 << US_TRIG_P));  // TRIG LOW（約 300μs 後）

        // 等待 ECHO 上升沿（最多等 5ms）
        unsigned long t = micros();
        while (digitalRead(US_ECHO_PIN) == LOW) {
            if (micros() - t > 5000) return -1.0f;  // 超時
        }

        // 量測 ECHO 高電位持續時間
        unsigned long start = micros();
        while (digitalRead(US_ECHO_PIN) == HIGH) {
            if (micros() - start > 25000) return -1.0f;  // 超過 4m
        }
        unsigned long duration = micros() - start;

        // 換算距離：音速 340m/s = 0.034 cm/μs，除以 2（來回）
        return (float)duration * 0.034f / 2.0f;
    }

    // ── 多次量測取中位數（濾除尖波雜訊）────────────────
    // samples：取樣次數（建議 3 或 5）
    float measureMedianCm(int samples = 3) {
        float readings[5];
        int count = min(samples, 5);

        for (int i = 0; i < count; i++) {
            readings[i] = measureCm();
            delay(30);  // HC-SR04 每次量測至少間隔 30ms
        }

        // 簡單排序（小陣列用插入排序）
        for (int i = 1; i < count; i++) {
            float key = readings[i];
            int j = i - 1;
            while (j >= 0 && readings[j] > key) {
                readings[j + 1] = readings[j--];
            }
            readings[j + 1] = key;
        }

        return readings[count / 2];  // 回傳中位數
    }

    // ── 判斷是否有人（距離 ≤ 閾值）──────────────────────
    bool isPersonNearby(float thresholdCm = WAKE_DISTANCE_CM) {
        float d = measureMedianCm(3);
        if (d < 0) return false;   // 量測失敗視為無人
        _lastDistance = d;
        return d <= thresholdCm;
    }

    // ── 取得最後一次量測距離 ──────────────────────────
    float getLastDistance() { return _lastDistance; }

private:
    float _lastDistance = 999.0f;
};
