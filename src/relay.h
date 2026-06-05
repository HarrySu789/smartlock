// src/relay.h
#pragma once

#include <Arduino.h>
#include "config.h"

// 繼電器邏輯說明（ HIGH 觸發 + NO 常開接線）：
//   RELAY_PIN = HIGH → 繼電器動作（通電）→ NO 閉合 → 門打開（解鎖）
//   RELAY_PIN = LOW  → 繼電器不動作（斷電）→ NO 打開 → 門鎖緊
// 重點：請根據您的硬體設備調整此邏輯

bool relayUnlocked = false;
unsigned long relayOpenTime = 0;

void initRelay() {
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);    // 確保啟動時門是鎖緊的（LOW = 不動作 = 鎖門）
    Serial.println("✅ 繼電器初始化（門已鎖緊）");
}

// 開鎖（持續 durationMs 毫秒後自動鎖門）
// durationMs = 0 表示永久打開直到呼叫 lockDoor()
void unlockDoor(unsigned long durationMs = UNLOCK_DURATION_MS) {
    digitalWrite(RELAY_PIN, HIGH);   // HIGH = 繼電器動作 = NO 閉合 = 開門
    relayUnlocked  = true;
    relayOpenTime  = millis();
    Serial.printf("🔓 開鎖 %lu ms\n", durationMs);
}

// 手動鎖門
void lockDoor() {
    digitalWrite(RELAY_PIN, LOW);    // LOW = 繼電器不動作 = NO 打開 = 鎖門
    relayUnlocked = false;
    Serial.println("🔒 已鎖門");
}

// 自動鎖門計時器（在 loop 中每次呼叫）
void updateRelay() {
    if (relayUnlocked && UNLOCK_DURATION_MS > 0) {
        if (millis() - relayOpenTime >= UNLOCK_DURATION_MS) {
            lockDoor();
        }
    }
}

bool isDoorUnlocked() {
    return relayUnlocked;
}

// 取得剩餘開鎖秒數（用於 OLED 倒數顯示）
int getUnlockRemainingSeconds() {
    if (!relayUnlocked) return 0;
    long remaining = (long)UNLOCK_DURATION_MS -
                     (long)(millis() - relayOpenTime);
    return (int)(remaining / 1000) + 1;
}