#pragma once

#include <Arduino.h>

#define BATT_ADC_PIN    1    // GPIO1 (D0)
#define LOW_BATTERY_THRESHOLD 20  // 電量 < 20% 時警告

// 請依照你的實測校正倍率（如果 3.127 量出來很準就不用動）
const float VOLTAGE_DIVIDER_RATIO = 3.177; 

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
    
    // EMA 濾波器的歷史變數
    float smoothedVoltage = -1.0f; 
    
    void begin() {
        pinMode(BATT_ADC_PIN, INPUT);
        analogSetAttenuation(ADC_11db);
        Serial.println("✅ 電池監控初始化完成 (支援 8.4V 架構 + EMA 濾波)");
    }
    
    Status getStatus(bool forceUpdate = false) {
        // 放寬到 10 秒更新一次就好，避免太頻繁讀取
        if (!forceUpdate && lastCheck != 0 && (millis() - lastCheck < 10000)) {
            return cachedStatus;
        }
        lastCheck = millis();
        
        // 1. 增加採樣次數到 64 次以消除高頻雜訊
        long sum_mv = 0;
        const int samples = 64;
        for (int i = 0; i < samples; i++) {
            sum_mv += analogReadMilliVolts(BATT_ADC_PIN);
            delay(1); 
        }
        
        // ... 前面的採樣與電壓計算維持不變 ...
        float adcVoltage = (float)(sum_mv / samples) / 1000.0f;
        float currentVoltage = adcVoltage * VOLTAGE_DIVIDER_RATIO;
        
        // 2. 導入 EMA 濾波與「換電池突變偵測」
        // 計算當前真實電壓與歷史平滑電壓的差距的絕對值
        float voltageDiff = abs(currentVoltage - smoothedVoltage);

        if (smoothedVoltage < 0 || voltageDiff > 0.4f) {
            // 觸發條件：第一次開機，或是偵測到大於 0.4V 的瞬間跳變 (代表更換電池)
            // 動作：直接「重置」濾波器，瞬間跟上真實電壓，不經過平滑計算
            smoothedVoltage = currentVoltage; 
            if (smoothedVoltage > 0) {
                Serial.printf("⚡ 偵測到更換電池 (跳變 %.2fV)，重置電量顯示！\n", voltageDiff);
            }
        } else {
            // 觸發條件：差距在 0.4V 以內 (正常的 WiFi 壓降或緩慢放電)
            // 動作：套用平滑濾波，Alpha = 0.15 抵抗高頻雜訊
            smoothedVoltage = (currentVoltage * 0.15f) + (smoothedVoltage * 0.85f);
        }
        
        // ... 下方的換算百分比維持不變 ...
        
        // 將平滑後的原始電壓，丟進校正函數，得出絕對精準的真實電壓
        float finalRealVoltage = calibrateVoltage(smoothedVoltage);
        
        // 使用「校正後的真實電壓」來計算百分比
        int pct = voltageToPercent(finalRealVoltage);
        bool isCharging = false; 
        
        cachedStatus = {
            finalRealVoltage,
            pct,
            isCharging,
            (pct < LOW_BATTERY_THRESHOLD)
        };
        
        Serial.printf("🔋 [原始平滑: %.2fV] 函數校正後真實電壓: %.2fV, %d%%\n", smoothedVoltage, finalRealVoltage, pct);
        
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
     // 分段線性插值校正函數 (專屬硬體校正模型)
     float calibrateVoltage(float rawMonitorV) {
         // 實測數據矩陣：x 為 Monitor 原始顯示電壓，y 為三用電表真實電壓
         float x1 = 7.08, y1 = 7.55; 
         float x2 = 7.42, y2 = 7.83; 
         float x3 = 7.59, y3 = 7.96; 

         // 1. 低於點 1：使用線段 1-2 的斜率進行向下外推
         if (rawMonitorV <= x1) {
             float slope1 = (y2 - y1) / (x2 - x1);
             return y1 - (x1 - rawMonitorV) * slope1;
         }
         
         // 2. 高於點 3：使用線段 2-3 的斜率進行向上外推
         if (rawMonitorV >= x3) {
             float slope2 = (y3 - y2) / (x3 - x2);
             return y3 + (rawMonitorV - x3) * slope2;
         }

         // 3. 落在區間內：進行精確的內插計算
         if (rawMonitorV <= x2) {
             return y1 + (rawMonitorV - x1) * (y2 - y1) / (x2 - x1);
         } else {
             return y2 + (rawMonitorV - x2) * (y3 - y2) / (x3 - x2);
         }
     }

     int voltageToPercent(float v) {
        // 根據你實測的專屬電池放電曲線 (100%=8.05V, 50%=7.76V)
        if (v >= 8.05f) return 100;
        if (v >= 7.76f) return 50 + (v - 7.76f) / (8.05f - 7.76f) * 50; // 50%~100% 區間
        if (v >= 7.40f) return 20 + (v - 7.40f) / (7.76f - 7.40f) * 30; // 20%~50% 區間
        if (v >= 6.80f) return  5 + (v - 6.80f) / (7.40f - 6.80f) * 15; // 5%~20% 區間
        return 0; // 低於 6.80V 視為沒電，應盡速充電
    }
};