// src/battery.h
#pragma once

#include <Arduino.h>

#define BATT_ADC_PIN    8    // GPIO8 (D9) ← 分壓器輸出
#define BATT_CHRG_PIN   7    // GPIO7 (D8) ← TP4056 CHRG 腳位
#define ADC_RESOLUTION  4095 // ESP32 12-bit ADC
#define ADC_REF_VOLTAGE 3.3f // 參考電壓

// 電池電壓對應的 ADC 值（分壓比 0.5）
// ADC_VAL = (Battery_V / 2) / 3.3 * 4095
#define BATT_FULL_ADC   2607  // 4.2V 滿電
#define BATT_GOOD_ADC   2482  // 4.0V 良好
#define BATT_MID_ADC    2296  // 3.7V 中等
#define BATT_LOW_ADC    2172  // 3.5V 偏低
#define BATT_CRIT_ADC   2048  // 3.3V 嚴重（需充電）

// 低電量警告閾值
#define LOW_BATTERY_THRESHOLD 20  // 電量 < 20% 時警告

class BatteryMonitor {
public:
    struct Status {
        int rawADC;
        float voltage;
        int percentage;
        bool charging;
        bool lowBattery;
    };
    
    unsigned long lastCheck = 0;
    Status cachedStatus = {0, 0, 100, false, false};
    
    void begin() {
        pinMode(BATT_ADC_PIN, INPUT);
        pinMode(BATT_CHRG_PIN, INPUT_PULLUP);  // 上拉（CHRG 為 active low）
        
        // ESP32 ADC 校準（衰減 11dB 以支援到 3.3V 輸入）
        analogSetAttenuation(ADC_11db);
        
        Serial.println("✅ 電池監控初始化完成");
    }
    
    // 取得電池狀態（每 30 秒更新一次）
    Status getStatus(bool forceUpdate = false) {
        // 加入 lastCheck == 0 讓開機第一次一定會強迫讀取
        if (!forceUpdate && lastCheck != 0 && (millis() - lastCheck < 60000)) {
            return cachedStatus;
        }
        lastCheck = millis();
        
        // 取得電池狀態 (改良版)
        int sum_mv = 0;
        const int samples = 16;
        for (int i = 0; i < samples; i++) {
            // 直接讀取校準後的毫伏特 (mV)
            sum_mv += analogReadMilliVolts(BATT_ADC_PIN);
            delay(2);
        }
        
        // 計算實際電壓 (毫伏特平均值 / 1000 = 伏特，再乘以 2 補償分壓)
        float adcVoltage = (float)(sum_mv / samples) / 1000.0f;
        float battVoltage = adcVoltage * 2.0f;
        
        // 換算電量百分比（線性近似，18650 的放電曲線）
        int pct = voltageToPercent(battVoltage);
        
        // 充電狀態（TP4056 CHRG = LOW 時正在充電）
        bool isCharging = (digitalRead(BATT_CHRG_PIN) == LOW);
        
        // Calculate raw ADC value from sum_mv: (avg_mV / 3300) * 4095
        int avg_mv = sum_mv / samples;
        int rawADC = (avg_mv * ADC_RESOLUTION) / 3300;
        
        cachedStatus = {
            rawADC,
            battVoltage,
            pct,
            isCharging,
            (pct < LOW_BATTERY_THRESHOLD)
        };
        
        Serial.printf("電池：%.2fV, %d%%, %s\n",
                      battVoltage, pct,
                      isCharging ? "充電中" : "使用中");
        
        return cachedStatus;
    }
    
    // 格式化為顯示字串
    String toDisplayString() {
        auto s = getStatus();
        char buf[32];
        if (s.charging) {
            snprintf(buf, sizeof(buf), "CHG %.1fV", s.voltage);
        } else {
            snprintf(buf, sizeof(buf), "%d%% %.1fV", s.percentage, s.voltage);
        }
        return String(buf);
    }
    
private:
    int voltageToPercent(float v) {
        // 18650 放電特性（近似曲線）
        if (v >= 4.2f) return 100;
        if (v >= 4.0f) return 75 + (v - 4.0f) / (4.2f - 4.0f) * 25;
        if (v >= 3.7f) return 40 + (v - 3.7f) / (4.0f - 3.7f) * 35;
        if (v >= 3.5f) return 15 + (v - 3.5f) / (3.7f - 3.5f) * 25;
        if (v >= 3.3f) return  5 + (v - 3.3f) / (3.5f - 3.3f) * 10;
        return 0;
    }
};