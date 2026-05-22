# 🔐 智慧門鎖系統 - SmartLock

基於 **Seeed XIAO ESP32-S3 Sense** 的多功能智慧門鎖系統，整合人臉辨識、指紋辨識、密碼輸入、PIR 人體感測器喚醒、Telegram 遠端控制和天氣播報功能。

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Arduino-blue.svg)](https://platformio.org/)
[![ESP32-S3](https://img.shields.io/badge/ESP32-S3-Sense-green.svg)](https://www.seeedstudio.com/XIAO-ESP32S3-Sense-p-5676.html)

---

## ✨ 功能特色

### 🔐 多種解鎖方式
- **人臉辨識** - 本地端 MTCNN + MobileFaceNet 演算法，離線也能用
- **指紋辨識** - AS608 光學指紋模組，支援 127 枚指紋
- **密碼鍵盤** - 4×4 矩陣鍵盤，4~8 位數密碼
- **Telegram 遠端解鎖** - 隨時隨地傳送指令開門

### 💡 智慧節能
- **PIR 人體感測** - HC-SR501 偵測門外是否有人
- **自動休眠/喚醒** - 無人時進入省電模式，有人靠近自動喚醒
- **功耗大幅降低** - 休眠模式從 200mA 降至約 70mA

### 🌤️ 智慧生活
- **天氣播報** - 門內有人時自動播報天氣（5 分鐘冷卻）
- **天氣提醒** - 解鎖時顯示天氣資訊和出門建議
- **電量監控** - 18650 鋰電池供電，支援 UPS 不斷電

### 📱 遠端管理
- **Telegram Bot** - 狀態查詢、遠端開鎖、人臉/指紋管理
- **陌生人警報** - 偵測到未註冊人臉自動拍照通知

---

## 🛠️ 硬體需求

### 主要控制器
| 元件 | 型號 | 數量 |
|------|------|------|
| 開發板 | Seeed XIAO ESP32-S3 Sense | 1 |
| 鏡頭 | OV2640（內建）| 1 |

### 周邊模組
| 模組 | 型號 | 數量 | 備註 |
|------|------|------|------|
| OLED 顯示器 | SSD1306 128×64 I2C | 1 | |
| 指紋模組 | AS608 | 1 | UART 介面 |
| 矩陣鍵盤 | 4×4 | 1 | |
| I2C 擴充板 | PCF8574 | 2 | 鍵盤 + LED/PIR |
| 音頻擴大機 | MAX98357 | 1 | I2S 介面 |
| 繼電器模組 | 5V 單路 | 1 | 控制電磁鎖 |
| 人體紅外線感測 | HC-SR501 | 2 | 門外 + 門內 |
| 鋰電池 | 18650 3.7V | 1~2 | 備用電源 |
| 充電模組 | TP4056 Type-C | 1 | |
| UPS 模組 | IP5306 | 1 | |

---

## 📋 腳位配置

### XIAO ESP32-S3 腳位圖

```
        ┌─────────────────┐
        │  USB-C 供电     │
        └────────┬────────┘
                 │
    ┌────────────┼────────────┐
    │            │            │
  D0/TX      D1/BCLK      D2/LRC    ← I2S 音頻
  GPIO1      GPIO2        GPIO3
    │            │            │
    ▼            ▼            ▼
  繼電器      MAX98357    MAX98357

    ┌────────────┼────────────┐
    │            │            │
  D3/DIN      D4/SDA       D5/SCL    ← I2C (OLED + PCF8574)
  GPIO4       GPIO5        GPIO6
    │            │            │
    ▼            ▼            ▼
  MAX98357    I2C 匯流排    I2C 匯流排

    ┌────────────┼────────────┐
    │            │            │
  D6/TX        D7/RX       D8/ADC    ← 指紋 + 充電狀態
  GPIO43      GPIO44       GPIO7
    │            │            │
    ▼            ▼            ▼
  AS608 RX   AS608 TX    TP4056 CHRG

    ┌────────────┬────────────┐
    │            │            │
  D9/ADC       D10         3V3
  GPIO8        GPIO9        3.3V
    │            │            │
    ▼            ▼            ▼
  電池 ADC    門外 PIR     VCC
```

### 詳細腳位表

| 功能 | 腳位 | GPIO | 連接說明 |
|------|------|------|----------|
| I2C SDA | D4 | GPIO5 | OLED / PCF8574 #1 / #2 |
| I2C SCL | D5 | GPIO6 | OLED / PCF8574 #1 / #2 |
| AS608 TX | D6 | GPIO43 | → AS608 RX |
| AS608 RX | D7 | GPIO44 | ← AS608 TX |
| I2S BCLK | D1 | GPIO2 | MAX98357 BCLK |
| I2S LRC | D2 | GPIO3 | MAX98357 LRC |
| I2S DIN | D3 | GPIO4 | MAX98357 DIN |
| 繼電器 | D0 | GPIO1 | 繼電器 IN |
| 電池 ADC | D9 | GPIO8 | 分壓中點 |
| 充電狀態 | D8 | GPIO7 | TP4056 CHRG |
| 門外 PIR | D10 | GPIO9 | HC-SR501 OUT |

### PCF8574 #2（0x21）腳位

| 腳位 | 功能 |
|------|------|
| P0 | 綠色 LED（成功）|
| P1 | 紅色 LED（失敗/警報）|
| P2 | 蜂鳴器（選配）|
| P3 | 門內 PIR 輸入 |

---

## 📁 專案結構

```
smartlock/
├── platformio.ini           # PlatformIO 設定檔
├── partitions_smartlock.csv # Flash 分割表
├── README.md                # 本文件
├── .gitignore
├── data/                    # SPIFFS 檔案系統
│   └── tts/                 # 天氣播報音效
│       ├── sunny.wav
│       ├── cloudy.wav
│       └── rain.wav
├── include/                 # 標頭檔（預留）
├── lib/                     # 本地函式庫（預留）
├── src/                     # 原始碼
│   ├── main.cpp             # 主程式
│   ├── config.h             # 設定檔（WiFi、密碼、PIR）
│   ├── pir_sensor.h         # HC-SR501 PIR 感測器
│   ├── face_recognition.h   # 人臉辨識（MTCNN + MobileFaceNet）
│   ├── face_database.h      # 人臉資料庫（SPIFFS）
│   ├── face_manager.h       # 人臉管理
│   ├── oled_ui.h            # OLED UI 顯示
│   ├── battery.h            # 電池監控
│   ├── keypad.h             # 矩陣鍵盤掃描
│   ├── fingerprint.h        # AS608 指紋模組
│   ├── audio.h              # I2S 音頻播放
│   ├── relay.h              # 繼電器控制
│   ├── wifi_mgr.h           # WiFi 管理
│   ├── telegram_bot.h        # Telegram Bot
│   ├── weather.h            # 天氣 API
│   └── ultrasonic.h         # 超音波（預留）
└── test/                    # 測試檔案
```

---

## 🚀 快速開始

### 1. 環境設定

```bash
# 安裝 VS Code
# 安裝 PlatformIO IDE 擴充功能

# 克隆專案
git clone https://github.com/HarrySu789/smartlock.git
cd smartlock
```

### 2. 設定配置

編輯 `src/config.h` 中的敏感資訊：

```cpp
// ===== WiFi 設定 =====
#define WIFI_SSID      "你的WiFi名稱"
#define WIFI_PASS      "你的WiFi密碼"

// ===== Telegram Bot =====
#define BOT_TOKEN      "從@BotFather取得的Token"
#define CHAT_ID        "你的ChatID"

// ===== 天氣 API =====
#define OWM_API_KEY    "OpenWeatherMap API Key"
#define OWM_CITY       "Taipei"

// ===== 密碼設定（務必修改！）=====
#define DEFAULT_PASSWORD "1234"
#define ADMIN_PASSWORD   "9999"
```

### 3. 編譯與燒錄

```bash
# 使用 PlatformIO
pio run --target upload

# 或在 VS Code 中
# 點擊 PlatformIO 工具列的 Upload 按鈕
```

### 4. 上傳 SPIFFS（首次）

```bash
pio run --target uploadfs
```

---

## 📱 Telegram Bot 指令

| 指令 | 功能說明 |
|------|----------|
| `/unlock` | 遠端開鎖（5 秒）|
| `/status` | 系統狀態（IP、電量、人臉數）|
| `/weather` | 查詢天氣 |
| `/battery` | 電量資訊 |
| `/alarm_off` | 解除警報 |
| `/face_list` | 列出所有人臉 |
| `/face_enroll 姓名` | 新增人臉 |
| `/face_delete 姓名` | 刪除人臉 |
| `/face_deleteall` | 清除所有人臉 |
| `/set_password 新密碼` | 修改密碼 |
| `/help` | 顯示說明 |

> 💡 **提示**：休眠期間仍可使用 `/unlock`，系統會自動喚醒

---

## 🎯 使用方式

### 解鎖流程

```
┌─────────────────────────────────────────────────────────────┐
│                        智慧門鎖系統                           │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│   ┌─────────┐   ┌─────────┐   ┌─────────┐   ┌─────────┐   │
│   │ 人臉辨識 │   │ 指紋感測 │   │ 密碼輸入 │   │ Telegram│   │
│   └────┬────┘   └────┬────┘   └────┬────┘   └────┬────┘   │
│        │             │             │              │        │
│        └─────────────┴──────┬──────┴──────────────┘        │
│                              │                              │
│                              ▼                              │
│                      ┌─────────────┐                        │
│                      │   驗證成功   │                        │
│                      └──────┬──────┘                        │
│                             │                               │
│        ┌────────────────────┼────────────────────┐          │
│        │                    │                    │          │
│        ▼                    ▼                    ▼          │
│   ┌─────────┐         ┌─────────┐         ┌─────────┐    │
│   │  開門   │         │  通知   │         │ 天氣顯示 │    │
│   │  5秒   │         │ Telegram│         │  提醒   │    │
│   └─────────┘         └─────────┘         └─────────┘    │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### 密碼輸入

```
鍵盤佈局：
┌─────┬─────┬─────┬─────┐
│  1  │  2  │  3  │  A  │  ← A = 進入管理模式
├─────┼─────┼─────┼─────┤
│  4  │  5  │  6  │  B  │
├─────┼─────┼─────┼─────┤
│  7  │  8  │  9  │  C  │  ← C = 指紋驗證
├─────┼─────┼─────┼─────┤
│  *  │  0  │  #  │  D  │  ← * = 清除 / # = 確認 / D = 刪一位
└─────┴─────┴─────┴─────┘
```

### 管理模式

1. 輸入管理員密碼（預設：`9999`）
2. 按 `A` 進入管理模式

| 按鍵 | 功能 |
|------|------|
| `1` | 新增人臉（透過 Telegram）|
| `2` | 新增指紋 |
| `3` | 列出已註冊人臉 |
| `4` | 刪除所有人臉 |
| `*` | 返回待機 |

---

## ⚡ PIR 節能機制

```
┌────────────────────────────────────────────────────────────┐
│                      系統狀態機                             │
├────────────────────────────────────────────────────────────┤
│                                                            │
│  ╔══════════════╗                                          │
│  ║  STATE_SLEEP ║  ← 休眠模式 (~70mA)                      │
│  ║  OLED 關閉    ║     PIR 每 500ms 檢測一次                │
│  ║  鏡頭暫停     ║                                          │
│  ╚══════╤═══════╝                                          │
│         │                                                   │
│    ┌────┴────┐                                             │
│    │         │                                             │
│ 門外 PIR  門內 PIR                                         │
│    │         │                                             │
│    │         └──────→ 天氣播報（5 分鐘冷卻）                 │
│    │                                                        │
│    └──────────→ 喚醒系統                                    │
│                     │                                       │
│                     ▼                                       │
│              ╔══════════════╗                               │
│              ║  STATE_IDLE  ║  ← 待機模式 (~200mA)         │
│              ║  全功能運作   ║                               │
│              ╚══════╤═══════╝                               │
│                     │                                       │
│         ┌───────────┴───────────┐                          │
│         │                       │                          │
│    無人 15秒                 有操作                          │
│         │                       │                          │
│         ▼                       │                          │
│  返回 SLEEP                     │                          │
│                                │                            │
│                                └────────────────────────────┘
│
└────────────────────────────────────────────────────────────┘
```

---

## 🔧 設定參數

在 `src/config.h` 中可調整：

```cpp
// PIR 感測設定
#define SLEEP_TIMEOUT_SEC  15      // 無人多久進入休眠（秒）
#define PIR_COOLDOWN_SEC   30      // 天氣播報冷卻時間（秒）

// 密碼設定
#define DEFAULT_PASSWORD   "1234"  // 開鎖密碼
#define ADMIN_PASSWORD     "9999"  // 管理員密碼
#define MAX_FAIL_ATTEMPTS  5       // 失敗次數上限

// 系統行為
#define UNLOCK_DURATION_MS 5000    // 開鎖持續時間（毫秒）
#define FACE_SCAN_INTERVAL_MS 300 // 人臉掃描間隔

// 電量警告
#define LOW_BATTERY_PCT    20      // 低電量警告
#define CRITICAL_BATTERY_PCT 10    // 嚴重低電量
```

---

## ❓ 常見問題

### Q: 編譯失敗，找不到人臉辨識函式庫？
**A**: 執行 `pio run --target clean` 後重新編譯

### Q: OLED 顯示異常？
**A**: 確認 I2C 接線，檢查 OLED 地址是否為 `0x3C`

### Q: 人臉辨識準確率低？
**A**: 
- 增加登錄照片數量
- 登錄時拍攝不同角度
- 確保光線條件一致
- 考慮加裝紅外補光燈

### Q: PIR 誤觸發？
**A**: 
- 調整安裝位置，遠離窗戶和熱源
- PIR 約需 30 秒預熱穩定
- 確認 VCC 為 3.3V

### Q: 休眠時仍可遠端解鎖嗎？
**A**: 可以，WiFi 和 Telegram 在休眠期間持續運作

---

## 📄 授權

本專案供學習和研究使用。

---

## 🙏 致謝

- [esp-face](https://github.com/espressif/esp-face) - 本地人臉辨識框架
- [Universal-Arduino-Telegram-Bot](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot)
- [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306)
- [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio)

---

## 📝 版本資訊

| 版本 | 日期 | 說明 |
|------|------|------|
| V3.0 | 2026/05/15 | PIR 人體感測器版，加入節能喚醒機制 |
| V2.0 | 2025/xx/xx | 多種解鎖方式整合 |
| V1.0 | 2025/xx/xx | 初始版本 |

**開發環境**: PlatformIO + Arduino Framework  
**目標硬體**: Seeed XIAO ESP32-S3 Sense  
**Framework**: ESP-IDF / Arduino
