// src/battery.h
#pragma once

#include <Arduino.h>

#define BATT_ADC_PIN    8    // GPIO8 ← 接到 10k 與 4.7k 電阻的交界處 (測量點)
// 如果沒有 TP5100 這類的充電模組，你可以先把 CHRG_PIN 註解掉
// #define BATT_CHRG_PIN   7 
#define LOW_BATTERY_THRESHOLD 20  // 電量 < 20% 時警告

// 分壓電路參數 (8.4V -> 2.68V 安全降壓)
// R1 = 10k ohm (接電池正極), R2 = 4.7k ohm (接 GND)
const float VOLTAGE_DIVIDER_RATIO = (10000.0f + 4700.0f) / 4700.0f; // 約 3.127

class BatteryMonitor {
public:
    struct Status {
        float voltage;
        int percentage;
        bool charging;
        bool lowBattery;
    };
    
    unsigned long lastCheck = 0;
    Status cachedStatus = {0, 100, false, false};
    
    void begin() {
        pinMode(BATT_ADC_PIN, INPUT);
        
        // 如果有充電模組才啟用這行
        // pinMode(BATT_CHRG_PIN, INPUT_PULLUP);  
        
        // ESP32 ADC 校準（衰減 11dB 以支援到最高 3.1V 左右的輸入）
        analogSetAttenuation(ADC_11db);
        
        Serial.println("✅ 電池監控初始化完成 (支援 8.4V 架構)");
    }
    
    Status getStatus(bool forceUpdate = false) {
        if (!forceUpdate && lastCheck != 0 && (millis() - lastCheck < 2000)) {
            return cachedStatus;
        }
        lastCheck = millis();
        
        // 16 次採樣濾波 (極好的寫法！)
        int sum_mv = 0;
        const int samples = 16;
        for (int i = 0; i < samples; i++) {
            sum_mv += analogReadMilliVolts(BATT_ADC_PIN);
            delay(2);
        }
        
        // 計算測量點實際電壓 (V)
        float adcVoltage = (float)(sum_mv / samples) / 1000.0f;
        
        // 乘回分壓倍率，還原電池真實總電壓
        float battVoltage = adcVoltage * VOLTAGE_DIVIDER_RATIO;
        
        // 換算 2S 電池電量百分比
        int pct = voltageToPercent(battVoltage);
        
        // 如果有接充電模組的訊號腳位，可改為 digitalRead(BATT_CHRG_PIN) == LOW
        bool isCharging = false; 
        
        cachedStatus = {
            battVoltage,
            pct,
            isCharging,
            (pct < LOW_BATTERY_THRESHOLD)
        };
        
        Serial.printf("🔋 電池狀態: %.2fV, %d%%\n", battVoltage, pct);
        
        return cachedStatus;
    }
    
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
        // 2S (串聯兩顆) 18650 的放電特性曲線對應
        if (v >= 8.4f) return 100;
        if (v >= 8.0f) return 75 + (v - 8.0f) / (8.4f - 8.0f) * 25;
        if (v >= 7.4f) return 40 + (v - 7.4f) / (8.0f - 7.4f) * 35;
        if (v >= 7.0f) return 15 + (v - 7.0f) / (7.4f - 7.0f) * 25;
        if (v >= 6.6f) return  5 + (v - 6.6f) / (7.0f - 6.6f) * 10;
        return 0; // 低於 6.6V (單顆低於 3.3V) 視為沒電，應盡速充電以保護電池
    }
};