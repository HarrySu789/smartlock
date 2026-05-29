# 🔐 SmartLock - XIAO ESP32-S3 Sense 智慧門鎖系統

[![PlatformIO](https://img.shields.io/badge/PlatformIO-Arduino-blue.svg)](https://platformio.org/)
[![ESP32-S3](https://img.shields.io/badge/ESP32-S3-Sense-green.svg)](https://www.seeedstudio.com/XIAO-ESP32S3-Sense-p-5676.html)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

> 基於 **Seeed XIAO ESP32-S3 Sense** 的多功能智慧門鎖系統，整合人臉辨識、指紋辨識、密碼輸入、PIR 人體感測器喚醒、Telegram 遠端控制和天氣播報功能。

---

## 📋 目錄

- [功能特色](#-功能特色)
- [硬體需求](#-硬體需求)
- [系統架構](#-系統架構)
- [軟體架構](#-軟體架構)
- [腳位配置](#-腳位配置)
- [專案結構](#-專案結構)
- [快速開始](#-快速開始)
- [Telegram Bot 指令](#-telegram-bot-指令)
- [使用方式](#-使用方式)
- [PIR 節能機制](#-pir-節能機制)
- [電路接線圖](#-電路接線圖)
- [設定參數](#-設定參數)
- [燒錄指南](#-燒錄指南)
- [常見問題](#-常見問題)
- [版本資訊](#-版本資訊)
- [授權與致謝](#-授權與致謝)

---

## ✨ 功能特色

### 🔐 多種解鎖方式

| 解鎖方式 | 說明 | 硬體元件 |
|---------|------|----------|
| **人臉辨識** | 本地端 MTCNN + MobileFaceNet 演算法，離線也能用 | OV2640 鏡頭（內建）|
| **指紋辨識** | AS608 光學指紋模組，支援 127 枚指紋 | AS608 指紋感測器 |
| **密碼鍵盤** | 4×4 矩陣鍵盤，4~8 位數密碼 | PCF8574 I/O 擴充 + 鍵盤 |
| **Telegram 遠端解鎖** | 隨時隨地傳送指令開門 | WiFi + Telegram Bot |

### 💡 智慧節能

| 功能 | 說明 | 節能效果 |
|------|------|----------|
| **PIR 人體感測** | HC-SR501 偵測門外是否有人，喚醒系統 | 避免無效待機 |
| **門內 PIR 觸發** | 偵測室內有人時自動播報天氣 | 智慧生活 |
| **自動休眠/喚醒** | 無人時進入省電模式，有人靠近自動喚醒 | 功耗從 200mA 降至 ~70mA |
| **OLED 動態關閉** | 休眠時關閉顯示器，啟動時自動點亮 | 延長電池壽命 |

### 🌤️ 智慧生活

| 功能 | 說明 | 觸發條件 |
|------|------|----------|
| **天氣播報** | 門內有人時自動播報天氣語音 | PIR_COOLDOWN_SEC 冷卻時間 |
| **天氣提醒** | 解鎖時顯示天氣資訊和出門建議 | 解鎖成功後 |
| **電量監控** | 18650 鋰電池供電，即時顯示電量 | 每 5 分鐘檢查 |
| **低電量警告** | 電量低於 20% 時播放警告音效 | 自動觸發 |

### 📱 遠端管理

| 功能 | 說明 | 說明 |
|------|------|------|
| **Telegram Bot** | 狀態查詢、遠端開鎖、人臉/指紋管理 | 24/7 隨時存取 |
| **陌生人警報** | 偵測到未註冊人臉自動拍照通知 | 安全性提升 |
| **密碼遠端修改** | 透過 Telegram 更改開鎖密碼 | NVS 持久化儲存 |
| **開機通知** | 系統啟動後自動發送狀態訊息 | 監控系統上線 |

### 🔊 音效系統

| 音效 | 觸發時機 | 實作方式 |
|------|----------|----------|
| 開機音效 | 系統啟動完成 | 合成音調旋律 |
| 解鎖成功 | 人臉/指紋/密碼驗證通過 | 合成音調旋律 |
| 解鎖失敗 | 驗證失敗 | 合成音調旋律 |
| 警報聲 | 連續失敗 5 次 | 緊急音效 |
| 天氣播報 | 門內 PIR 觸發 | WAV 檔案播放（SPIFFS）|
| 低電量警告 | 電量低於 20% | 合成音調旋律 |

---

## 🛠️ 硬體需求

### 主要控制器

| 元件 | 型號 | 數量 | 備註 |
|------|------|------|------|
| 開發板 | Seeed XIAO ESP32-S3 Sense | 1 | 含 8MB Flash + 8MB PSRAM |
| 鏡頭 | OV2640（內建）| 1 | 240×240 RGB565 拍攝 |
| 音頻放大器 | MAX98357A I2S | 1 | I2S 介面 |
| 喇叭 | 4Ω 3W | 1 | 直徑 40~50mm |

### 周邊模組

| 模組 | 型號 | 數量 | 備註 |
|------|------|------|------|
| OLED 顯示器 | SSD1306 128×64 I2C | 1 | I2C 地址 0x3C |
| 指紋模組 | AS608 | 1 | UART 介面，57600 baud |
| 矩陣鍵盤 | 4×4 薄膜鍵盤 | 1 | 8 條訊號線 |
| I2C 擴充板 | PCF8574 | 2 | 鍵盤 (0x20) + 狀態 (0x21) |
| 繼電器模組 | 5V 單路（光耦隔離）| 1 | 控制電磁鎖 |
| 人體紅外線感測 | HC-SR501 | 2 | 門外 GPIO + 門內 PCF8574 |
| 鋰電池 | 18650 3.7V 2000mAh+ | 1~2 | 備用電源 |

### 電源供應

| 元件 | 規格 | 用途 |
|------|------|------|
| 12V 變壓器 | 12V 2A | 電磁鎖供電 |
| DC-DC 降壓 | 12V → 5V 2A | 系統供電 |
| 18650 鋰電池 | 3.7V 2000mAh+ | 斷電備援 |

### 電子零件

| 元件 | 規格 | 數量 | 用途 |
|------|------|------|------|
| 電阻 | 4.7kΩ 1/4W | 4 | I2C 上拉電阻 |
| 電阻 | 100kΩ 1/4W | 2 | 電池分壓器 |
| 電阻 | 10kΩ 1/4W | 1 | CHRG 上拉 |
| 電阻 | 330Ω 1/4W | 2 | LED 限流 |
| LED | 5mm 綠色 | 1 | 狀態指示 |
| LED | 5mm 紅色 | 1 | 警報指示 |
| 電容 | 100μF 10V 電解 | 2 | 電源去耦 |
| 電容 | 100nF 陶瓷 | 4 | ADC/I2C 去耦 |

---

## 🏗️ 系統架構

### 硬體連接架構

```
                            ┌─────────────────────────────────────────────────────┐
                            │              XIAO ESP32-S3 Sense                   │
                            │                                                     │
   ┌─────────┐              │  GPIO5(SDA) ─────────────────────────────────────► │
   │  OLED   │◄─────────────┤  GPIO6(SCL) ─────────────────────────────────────► │
   │ 0x3C    │              │                          ┌──────── I2C Bus ────┐   │
   └─────────┘              │                          │                     │   │
   ┌─────────┐              │                       ┌──▼──────┐  ┌──────────▼┐  │
   │PCF8574  │◄─────────────┤                       │PCF8574  │  │ PCF8574   │  │
   │鍵盤0x20 │              │                       │鍵盤0x20 │  │ LED 0x21  │  │
   └────┬────┘              │                       └──┬──────┘  └──────────┬┘  │
        │                   │                          │(8腳)               │    │
   ┌────▼────┐              │                     ┌────▼────┐           ┌───▼──┐ │
   │ 4×4鍵盤 │              │                     │ 4×4鍵盤 │           │LED/  │ │
   └─────────┘              │                     └─────────┘           │蜂鳴器│ │
                            │                                           └──────┘ │
   ┌─────────┐              │  GPIO43(TX)──────────────────────────────────────► │
   │  AS608  │◄─────────────┤  GPIO44(RX)◄──────────────────────────────────── │
   │指紋模組 │              │                                                     │
   └─────────┘              │  GPIO2(BCLK)─────────────────────────────────────► │
                            │  GPIO3(LRC)──────────────────────────────────────► │
   ┌─────────┐              │  GPIO4(DIN)──────────────────────────────────────► │
   │MAX98357 │◄─────────────┤                                                     │
   │ + 喇叭  │              │  GPIO1──────────────────────────────────► 繼電器  │
                            └─────────────────────────────────────────────────────┘
                                                    │
                            ┌───────────────────────▼──────────────────────────┐
                            │                     繼電器                          │
                            │  NC ────── 12V 電源 ────── 電磁鎖（NC 常閉型）       │
                            │  COM ───── GND                                    │
                            └──────────────────────────────────────────────────────┘
```

### 電源架構

```
                    ┌─────────────────────────────────────────────────────────────┐
                    │                        正常供電                              │
                    │  USB/變壓器 → TP4056 → [對電池充電] → IP5306 → 5V → 系統   │
                    └─────────────────────────────────────────────────────────────┘
                                            │
                    ┌────────────────────────┴────────────────────────┐
                    │                  斷電後（自動切換）                   │
                    │  18650 鋰電池 → IP5306 → 5V → 系統（<20ms 切換）    │
                    └────────────────────────────────────────────────────┘
                                            │
                    ┌────────────────────────┴────────────────────────┐
                    │              電量監控（分壓電路）                 │
                    │  電池(+) ──── 100kΩ ──── ESP32 GPIO8(ADC)       │
                    │                   │                              │
                    │                   └─── 100kΩ ──── GND            │
                    │                   分壓比 0.5，最大電壓 2.1V      │
                    └─────────────────────────────────────────────────┘
```

### 軟體系統架構

```
┌──────────────────────────────────────────────────────────────────────┐
│                            main.cpp                                  │
├──────────────────────────────────────────────────────────────────────┤
│  setup()                                                             │
│  ├── Serial.begin(115200)                                            │
│  ├── Preferences.begin() → 讀取儲存的密碼                            │
│  ├── SPIFFS.begin() → 掛載檔案系統                                  │
│  ├── Wire.begin() → I2C 初始化                                       │
│  ├── OledUI.begin() → OLED 顯示器初始化                              │
│  ├── pcf8574_init() → I2C 擴充初始化                                 │
│  ├── pinMode(RELAY_PIN, OUTPUT) → 繼電器初始化                       │
│  ├── BatteryMonitor.begin() → 電池監控初始化                         │
│  ├── PIRSensor.begin() → PIR 感測器初始化                            │
│  ├── FaceRecognitionSystem.initCamera() → 鏡頭初始化                 │
│  ├── FaceDatabase.begin() → SPIFFS 人臉資料庫                        │
│  ├── initFingerprint() → 指紋模組初始化                              │
│  ├── initAudio() → I2S 音頻初始化                                    │
│  ├── connectWiFi() → WiFi 連線                                       │
│  ├── syncTime() → NTP 時間同步                                       │
│  └── getWeather() → 天氣資料取得                                     │
├──────────────────────────────────────────────────────────────────────┤
│  loop()                                                              │
│  ├── maintainWiFi() → WiFi 自動重連（每 30 秒）                      │
│  ├── handleTelegramCommands() → Telegram Bot 輪詢（每 3 秒）         │
│  ├── getWeather() → 天氣更新（每 10 分鐘）                           │
│  ├── BatteryMonitor → 電量檢查（每 5 分鐘）                         │
│  └── switch(currentState)                                            │
│      ├── STATE_SLEEP → handleSleep()                                 │
│      ├── STATE_IDLE → handleIdle()                                   │
│      ├── STATE_UNLOCKED → handleUnlocked()                          │
│      ├── STATE_ALARM → handleAlarm()                                 │
│      └── STATE_FACE_MGMT → handleFaceMgmt()                          │
└──────────────────────────────────────────────────────────────────────┘
```

### 狀態機架構

```
                    ┌──────────────────────────────────────────────────────┐
                    │                      系統狀態機                       │
                    ├──────────────────────────────────────────────────────┤
                    │                                                      │
                    │   ╔═══════════════════╗                              │
                    │   ║   STATE_SLEEP     ║  ← 休眠模式 (~70mA)          │
                    │   ║  • OLED 關閉       ║     每 500ms PIR 檢測        │
                    │   ║  • 鏡頭暫停        ║     WiFi 維持                │
                    │   ║  • 人臉辨識暫停    ║                              │
                    │   ╚══════╤════════════╝                              │
                    │          │                                           │
                    │    ┌─────┴─────┐                                      │
                    │    │           │                                      │
                    │ 門外 PIR   門內 PIR                                   │
                    │    │           │                                      │
                    │    │           └──→ 天氣播報（5 分鐘冷卻）             │
                    │    │                                               │
                    │    └──────────→ 喚醒系統                              │
                    │                     │                                │
                    │                     ▼                                │
                    │              ╔═══════════════╗                        │
                    │              ║  STATE_IDLE  ║  ← 待機模式 (~200mA)  │
                    │              ║  • OLED 顯示  ║                        │
                    │              ║  • 人臉辨識    ║                        │
                    │              ║  • 指紋掃描    ║                        │
                    │              ║  • 鍵盤輸入    ║                        │
                    │              ╚══════╤════════╝                        │
                    │                     │                                │
                    │    ┌────────────────┼────────────────┐              │
                    │    │                │                │              │
                    │ 密碼錯誤       驗證成功         無操作             │
                    │  5次           STATE_UNLOCKED  15秒             │
                    │    │                │                │              │
                    │    ▼                │                ▼              │
                    │ ╔════════════╗       │        返回 STATE_SLEEP      │
                    │ ║STATE_ALARM║       │                              │
                    │ ║ • 警報聲   ║       │                              │
                    │ ║ • 閃爍LED  ║       │                              │
                    │ ╚════════════╝       │                              │
                    │                       │                              │
                    │              ╔═══════════════╗                       │
                    │              ║STATE_FACE_MGMT║  ← 管理模式          │
                    │              ║  • 新增人臉    ║                        │
                    │              ║  • 新增指紋    ║                        │
                    │              ║  • 刪除人臉    ║                        │
                    │              ╚═══════════════╝                        │
                    │                                                      │
                    └──────────────────────────────────────────────────────┘
```

---

## 📂 軟體架構

### 人臉辨識流程（MTCNN + MobileFaceNet）

```
┌─────────────────────────────────────────────────────────────────────────┐
│                         人臉辨識三階段管線                               │
├─────────────────────────────────────────────────────────────────────────┤
│                                                                         │
│  ┌─────────────┐     ┌─────────────┐     ┌─────────────────────┐    │
│  │   OV2640    │     │    MTCNN     │     │     MobileFaceNet    │    │
│  │   鏡頭擷取   │────▶│   PNet 偵測   │────▶│     特徵向量萃取     │    │
│  │  240×240    │     │   (~50ms)    │     │     (~300ms)         │    │
│  │  RGB565     │     └─────────────┘     └──────────┬──────────┘    │
│  └─────────────┘                                       │               │
│                                                         │               │
│                        ┌────────────────────────────────┘               │
│                        ▼                                                │
│              ┌─────────────────────┐                                     │
│              │    特徵比對          │                                     │
│              │  • 餘弦相似度       │                                     │
│              │  • 閾值 0.6        │                                     │
│              │  • 回傳人員名稱     │                                     │
│              └──────────┬──────────┘                                     │
│                         │                                                │
│            ┌────────────┴────────────┐                                  │
│            │                         │                                  │
│            ▼                         ▼                                  │
│  ┌─────────────────┐       ┌─────────────────┐                         │
│  │   已註冊人臉    │       │    陌生人       │                         │
│  │  → 開鎖成功    │       │  → 陌生人警報    │                         │
│  │  → 綠燈        │       │  → 拍照通知      │                         │
│  │  → Telegram    │       │                 │                         │
│  └─────────────────┘       └─────────────────┘                         │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘
```

### 模組依賴關係

```
┌─────────────────────────────────────────────────────────────┐
│                        main.cpp                            │
│                      （主程式入口）                          │
└──────────────────────────┬──────────────────────────────────┘
                           │ #include
           ┌───────────────┼───────────────┬───────────────┐
           │               │               │               │
           ▼               ▼               ▼               ▼
    ┌────────────┐  ┌───────────┐  ┌──────────┐  ┌──────────┐
    │  config.h  │  │ oled_ui.h │  │  audio.h │  │   wifi_mgr.h   │
    │ （設定檔） │  │ （顯示）  │  │ （音效） │  │ （WiFi）  │
    └────────────┘  └───────────┘  └──────────┘  └──────────┘
           │               │               │               │
           ▼               ▼               ▼               ▼
    ┌────────────┐  ┌───────────┐  ┌──────────┐  ┌──────────┐
    │ pir_sensor.h │ │  keypad.h │  │ battery.h │  │ weather.h │
    │ （PIR）    │  │ （鍵盤）  │  │ （電池）  │  │ （天氣）  │
    └────────────┘  └───────────┘  └──────────┘  └──────────┘
           │               │               │
           ▼               ▼               ▼
    ┌────────────┐  ┌───────────┐  ┌──────────┐
    │ relay.h    │  │fingerprint.h│ │telegram_bot.h│
    │ （繼電器） │  │ （指紋）  │  │ （Telegram）│
    └────────────┘  └───────────┘  └──────────┘
           │               │               │
           └───────────────┼───────────────┘
                           │
                           ▼
    ┌──────────────────────────────────────┐
    │      face_recognition.h              │
    │   （人臉辨識 + 人臉資料庫管理）        │
    │                                      │
    │  • FaceRecognitionSystem              │
    │  • FaceDatabase                       │
    │  • FaceManager                        │
    └──────────────────────────────────────┘
```

---

## 📋 腳位配置

### XIAO ESP32-S3 Sense 腳位圖

```
            USB-C
     ┌──────[  ]──────┐
   D0│GPIO1           │GPIO43 D6   ← AS608 TX
   D1│GPIO2           │GPIO44 D7   ← AS608 RX
   D2│GPIO3           │GPIO45 D8   ← 充電狀態
   D3│GPIO4           │GPIO48 D9   ← 保留
   D4│GPIO5(SDA)      │3.3V
   D5│GPIO6(SCL)      │GND
   D6│GPIO43(TX)      │5V
   D7│GPIO44(RX)      │GND
     │                │
     │   [BOOT] [RST] │
     └────────────────┘
          底部有鏡頭排線連接器
```

### 詳細腳位表

| 功能 | XIAO 腳位 | GPIO | 說明 |
|------|----------|------|------|
| I2C SDA | D4 | GPIO5 | OLED / PCF8574 #1 / #2 共用 |
| I2C SCL | D5 | GPIO6 | OLED / PCF8574 #1 / #2 共用 |
| AS608 TX | D6 | GPIO43 | 連接 AS608 RXD |
| AS608 RX | D7 | GPIO44 | 連接 AS608 TXD |
| I2S BCLK | D1 | GPIO2 | MAX98357 BCLK |
| I2S LRC | D2 | GPIO3 | MAX98357 LRC |
| I2S DIN | D3 | GPIO4 | MAX98357 DIN |
| 繼電器 | D0 | GPIO1 | HIGH=鎖門 / LOW=開門 |
| 電池 ADC | D9 | GPIO8 | 分壓中點（0.5倍）|
| 充電狀態 | D8 | GPIO7 | TP4056 CHRG，低=充電中 |
| 門外 PIR | D10 | GPIO9 | HC-SR501 OUT |

### PCF8574 I2C 地址設定

**PCF8574 #1（地址 0x20）— 鍵盤掃描**

| 焊點 | 設定 | 地址位元 |
|------|------|----------|
| A0 | GND | 0 |
| A1 | GND | 0 |
| A2 | GND | 0 |
| **最終地址** | | **0x20** |

| PCF8574 #1 腳位 | 功能 | 說明 |
|-----------------|------|------|
| P0 | Row 1 | 鍵盤第 1 條線 |
| P1 | Row 2 | 鍵盤第 2 條線 |
| P2 | Row 3 | 鍵盤第 3 條線 |
| P3 | Row 4 | 鍵盤第 4 條線 |
| P4 | Col 1 | 鍵盤第 5 條線 |
| P5 | Col 2 | 鍵盤第 6 條線 |
| P6 | Col 3 | 鍵盤第 7 條線 |
| P7 | Col 4 | 鍵盤第 8 條線 |

**PCF8574 #2（地址 0x21）— 狀態指示**

| 焊點 | 設定 | 地址位元 |
|------|------|----------|
| A0 | VCC | 1 |
| A1 | GND | 0 |
| A2 | GND | 0 |
| **最終地址** | | **0x21** |

| PCF8574 #2 腳位 | 功能 | 說明 |
|-----------------|------|------|
| P0 | 綠色 LED | 解鎖成功指示（低電位亮）|
| P1 | 紅色 LED | 失敗/警報指示（低電位亮）|
| P2 | 蜂鳴器 | 選配（低電位響）|
| P3 | 門內 PIR | HC-SR501 訊號輸入 |

### 4×4 鍵盤按鍵佈局

```
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

---

## 📁 專案結構

```
smartlock/
├── README.md                    # 本文件（專案說明）
├── LICENSE                      # MIT 授權
├── platformio.ini              # PlatformIO 設定檔
├── partitions_smartlock.csv    # Flash 分割表
├── .gitignore                  # Git 忽略規則
│
├── data/                       # SPIFFS 檔案系統
│   └── tts/                    # 天氣播報音效
│       ├── sunny.wav           # 晴天音效
│       ├── cloudy.wav          # 多雲音效
│       └── rain.wav            # 雨天音效
│
├── include/                    # 標頭檔（預留）
│   └── README
│
├── lib/                       # 本地函式庫（預留）
│   └── README
│
├── src/                       # 原始碼
│   ├── main.cpp               # 主程式（狀態機、loop）
│   │
│   ├── config.h               # 全域設定檔
│   │                          # • WiFi SSID/密碼
│   │                          # • Telegram Bot Token
│   │                          # • 天氣 API Key
│   │                          # • 密碼設定
│   │                          # • 硬體腳位定義
│   │                          # • 功能開關
│   │                          # • 行為參數
│   │
│   ├── pir_sensor.h           # HC-SR501 PIR 人體感測器
│   │                          # • begin() - 初始化
│   │                          # • isOutsideDetected() - 門外偵測
│   │                          # • isInsideDetected() - 門內偵測
│   │
│   ├── face_recognition.h     # 人臉辨識系統（MTCNN + MobileFaceNet）
│   │                          # • initCamera() - 鏡頭初始化
│   │                          # • capture() - 擷取影像
│   │                          # • process() - 人臉偵測+辨識
│   │                          # • enroll() - 人臉登錄
│   │                          # • getList() - 取得人臉清單
│   │                          # • deleteAll() - 刪除所有人臉
│   │
│   ├── face_database.h        # SPIFFS 人臉資料庫
│   │                          # • begin() - 初始化
│   │                          # • save() - 儲存人臉
│   │                          # • loadAll() - 載入所有人臉
│   │                          # • remove() - 刪除指定人臉
│   │                          # • list() - 列出人臉
│   │
│   ├── face_manager.h         # 人臉管理邏輯
│   │                          # • enrollFace() - 互動式登錄
│   │                          # • restoreDatabase() - 還原資料庫
│   │
│   ├── oled_ui.h              # OLED 顯示介面（SSD1306 128×64）
│   │                          # • showIdle() - 待機畫面
│   │                          # • showVerifying() - 驗證中
│   │                          # • showUnlocked() - 解鎖成功
│   │                          # • showDenied() - 驗證失敗
│   │                          # • showAlarm() - 警報畫面
│   │                          # • showFaceMenu() - 人臉管理
│   │                          # • showPasswordInput() - 密碼輸入
│   │                          # • showMessage() - 通用訊息
│   │
│   ├── battery.h              # 電池監控系統
│   │                          # • begin() - 初始化
│   │                          # • getStatus() - 取得電量狀態
│   │                          # • voltageToPercent() - 電壓轉電量
│   │
│   ├── keypad.h               # 4×4 矩陣鍵盤掃描
│   │                          # • scanKeypad() - 掃描按鍵
│   │                          # • waitForKey() - 等待按鍵
│   │                          # • pcf8574_read/write() - I2C 操作
│   │
│   ├── fingerprint.h          # AS608 指紋模組
│   │                          # • initFingerprint() - 初始化
│   │                          # • verifyFingerprint() - 驗證指紋
│   │                          # • enrollFingerprint() - 登錄指紋
│   │                          # • deleteFingerprint() - 刪除指紋
│   │                          # • getNextFreeFingerprintID() - 找空位
│   │
│   ├── audio.h               # I2S 音頻播放（MAX98357）
│   │                          # • initAudio() - 初始化
│   │                          # • playNote() - 播放單音
│   │                          # • playSound() - 播放音效序列
│   │                          # • playSoundAsync() - 非同步播放
│   │                          # • playWavSync() - WAV 檔播放
│   │
│   ├── relay.h               # 繼電器控制（電磁鎖）
│   │                          # • initRelay() - 初始化
│   │                          # • unlockDoor() - 開鎖
│   │                          # • lockDoor() - 鎖門
│   │                          # • updateRelay() - 更新計時器
│   │
│   ├── wifi_mgr.h            # WiFi 管理
│   │                          # • connectWiFi() - 連線
│   │                          # • syncTime() - NTP 同步
│   │                          # • maintainWiFi() - 自動重連
│   │                          # • getCurrentDateTime() - 取得時間
│   │
│   ├── telegram_bot.h        # Telegram Bot 遠端控制
│   │                          # • initTelegramTLS() - TLS 初始化
│   │                          # • flushPendingTelegramMessages() - 清空堆積
│   │                          # • sendTelegramMessage() - 發送文字
│   │                          # • sendTelegramPhoto() - 發送照片
│   │                          # • handleTelegramCommands() - 處理指令
│   │
│   └── weather.h             # 天氣 API（OpenWeatherMap）
│                              # • getWeather() - 取得天氣資料
│                              # • getWeatherMessage() - 格式化訊息
│                              # • getWeatherShort() - 短版天氣
│
└── test/                     # 測試檔案
    └── README
```

---

## 🚀 快速開始

### 1. 環境設定

#### 安裝必要軟體

```bash
# 安裝 VS Code
https://code.visualstudio.com/download

# 安裝 PlatformIO IDE 擴充
# 在 VS Code 中：Ctrl+Shift+X → 搜尋 "PlatformIO IDE" → Install
```

#### 克隆專案

```bash
# 使用 Git Clone
git clone https://github.com/HarrySu789/smartlock.git
cd smartlock

# 或使用 GitHub CLI
gh repo clone HarrySu789/smartlock
```

### 2. 設定配置

複製 `src/config.h` 並建立 `src/config_local.h`（不會被 Git 追蹤）：

```bash
# 建立本地設定檔
copy src\config.h src\config_local.h
```

編輯 `src/config_local.h` 中的敏感資訊：

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
#define OWM_COUNTRY    "TW"

// ===== 密碼設定（務必修改！）=====
#define DEFAULT_PASSWORD "1234"   // 開鎖密碼
#define ADMIN_PASSWORD   "9999"   // 管理員密碼
```

> ⚠️ **安全提醒**：千萬不要將 `config_local.h` 提交到 Git！

### 3. 上傳 SPIFFS 檔案（首次設定）

```bash
# 使用 PlatformIO CLI
pio run --target uploadfs

# 或在 VS Code 中
# 1. 點擊 PlatformIO 工具列的 "Upload Filesystem Image"
```

### 4. 編譯與燒錄

```bash
# 編譯
pio run

# 燒錄（自動偵測 COM 埠）
pio run --target upload

# 燒錄並開啟序列埠監視器
pio run --target upload --monitor
```

---

## 📱 Telegram Bot 指令

### 核心功能

| 指令 | 功能說明 |
|------|----------|
| `/unlock` | 遠端開鎖（5 秒），休眠中也能喚醒 |
| `/status` | 系統狀態（IP、電量、人臉數、運行時間）|
| `/sleep` | 手動進入休眠模式 |
| `/alarm_off` | 解除警報 |

### 天氣與電量

| 指令 | 功能說明 |
|------|----------|
| `/weather` | 查詢目前天氣（溫度、濕度、風速）|
| `/battery` | 電量資訊（百分比、電壓、充電狀態）|

### 人臉管理

| 指令 | 功能說明 |
|------|----------|
| `/face_list` | 列出所有人臉 |
| `/face_enroll 姓名` | 新增人臉（30 秒內站到鏡頭前）|
| `/face_delete 姓名` | 刪除指定人臉 |
| `/face_deleteall` | 清除所有人臉（需確認）|

### 指紋管理

| 指令 | 功能說明 |
|------|----------|
| `/fp_enroll` | 指紋登錄說明 |
| `/fp_list` | 指紋列表說明 |

### 系統設定

| 指令 | 功能說明 |
|------|----------|
| `/set_password 新密碼` | 修改開鎖密碼（4~8 位）|
| `/hide_keyboard` | 隱藏 Telegram 鍵盤 |
| `/help` | 顯示完整說明 |

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
│   │  ~1秒   │   │  ~1秒   │   │  5秒    │   │  即時   │   │
│   └────┬────┘   └────┬────┘   └────┬────┘   └────┬────┘   │
│        │             │             │              │        │
│        └─────────────┴──────┬──────┴──────────────┘        │
│                             │                              │
│                             ▼                              │
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

### 密碼輸入流程

1. **待機畫面**：輸入數字（0-9）
2. **按 `#`** 確認密碼
3. **按 `*`** 清除輸入
4. **按 `D`** 刪除最後一位

### 管理模式

1. 輸入管理員密碼（預設：`9999`）
2. 按 `A` 進入管理模式

| 按鍵 | 功能 |
|------|------|
| `1` | 新增人臉（透過 Telegram）|
| `2` | 新增指紋（需在指紋感測器上操作）|
| `3` | 列出已註冊人臉 |
| `4` | 刪除所有人臉（需再按 `#` 確認）|
| `*` | 返回待機 |

### 指紋登錄流程

1. 進入管理模式（`9999` + `A`）
2. 按 `2` 選擇指紋登錄
3. 系統顯示指紋 ID
4. 第一次放上手指 → 等待確認
5. 移開手指
6. 第二次放上同一手指 → 等待確認
7. 登錄成功

---

## ⚡ PIR 節能機制

### 功耗對比

| 狀態 | 功耗 | 說明 |
|------|------|------|
| 休眠 (STATE_SLEEP) | ~70mA | OLED 關閉、鏡頭暫停、WiFi 維持 |
| 待機 (STATE_IDLE) | ~200mA | 全功能運作 |
| 警報 (STATE_ALARM) | ~220mA | LED 閃爍、蜂鳴器響 |

### PIR 感測器配置

| 位置 | 連接方式 | 用途 |
|------|----------|------|
| 門外 PIR | GPIO9 (D10) | 偵測門外人體，喚醒系統 |
| 門內 PIR | PCF8574 #2 P3 | 偵測室內人體，觸發天氣播報 |

### 節能策略

```
休眠狀態（~70mA）：
  • OLED 完全關閉（SSD1306 DISPLAYOFF）
  • 鏡頭停止取像（不分配 frame buffer）
  • 人臉辨識暫停
  • 每 500ms 只檢查一次 PIR
  • WiFi 和 Telegram Bot 維持運行

待機狀態（~200mA）：
  • OLED 顯示待機畫面
  • 人臉辨識每 300ms 執行一次
  • 指紋感測器持續輪詢
  • 鍵盤即時響應
  • PIR 每 500ms 檢查一次

節能效果：
  • 假設每天實際使用 1 小時，休眠 23 小時
  • 2000mAh 18650 鋰電池
  • 傳統模式：200mA × 24h = 4800mAh（需要 2.4 倍容量）
  • PIR 節能：200mA × 1h + 70mA × 23h = 1810mAh（可使用約 1 天）
```

---

## 🔌 電路接線圖

### I2C 匯流排（共用的 SDA/SCL）

```
ESP32 D4 (GPIO5, SDA) ──┬──── PCF8574 #1 SDA
                         ├──── PCF8574 #2 SDA
                         └──── OLED SDA

ESP32 D5 (GPIO6, SCL) ──┬──── PCF8574 #1 SCL
                         ├──── PCF8574 #2 SCL
                         └──── OLED SCL

3.3V ──── 4.7kΩ ──── SDA（上拉電阻）
3.3V ──── 4.7kΩ ──── SCL（上拉電阻）
```

### AS608 指紋模組

```
AS608          XIAO ESP32-S3
──────         ──────────────
VCC    ──────── 3.3V（重要：AS608 工作電壓 3.3V！）
GND    ──────── GND
TXD    ──────── D7 (GPIO44)  ← ESP32 RX
RXD    ──────── D6 (GPIO43)  ← ESP32 TX
```

### MAX98357A 音頻放大器

```
MAX98357A      XIAO ESP32-S3
───────        ──────────────
VIN    ──────── 5V（需要 5V 才有足夠音量）
GND    ──────── GND
BCLK   ──────── D1 (GPIO2)
LRC    ──────── D2 (GPIO3)
DIN    ──────── D3 (GPIO4)
```

### 繼電器模組與電磁鎖

```
繼電器控制側：
  VCC  ──────── 5V
  GND  ──────── GND（與 ESP32 共地）
  IN   ──────── D0 (GPIO1)

繼電器電源側（12V 高壓）：
  COM  ──────── 電磁鎖正極
  NC   ──────── 12V 電源正極

邏輯說明：
  GPIO1 = HIGH → 繼電器不動作 → NC連通COM → 電磁鎖通電 → 門鎖緊 ✓
  GPIO1 = LOW  → 繼電器動作   → NC斷開    → 電磁鎖斷電 → 門打開 ✓
  斷電時      → GPIO浮接      → 繼電器不動作 → 門鎖緊 ✓（安全）
```

### 電池監控電路

```
18650 鋰電池
     │
     ├─── 100kΩ ──┬─── ESP32 D9 (GPIO8) ADC
     │            │
     │            ├─── 100kΩ ──── GND
     │
     └─── 100nF ──┬─── GND（濾波）

TP4056 CHRG ──┬─── 10kΩ ──── 3.3V（上拉）
              │
              └─── ESP32 D8 (GPIO7)
              
CHRG = LOW  → 正在充電
CHRG = HIGH → 充飽或未接電源
```

---

## 🔧 設定參數

在 `src/config.h` 中可調整的參數：

### WiFi 設定

```cpp
#define WIFI_SSID          "你的WiFi名稱"
#define WIFI_PASS          "你的WiFi密碼"
#define WIFI_TIMEOUT_SEC   20          // 連線逾時秒數
```

### Telegram Bot 設定

```cpp
#define BOT_TOKEN          "從@BotFather取得的Token"
#define CHAT_ID            "你的ChatID"
#define BOT_POLL_MS        3000        // 輪詢間隔（毫秒），勿低於 1000
```

### 天氣 API 設定

```cpp
#define OWM_API_KEY        "OpenWeatherMap API Key"
#define OWM_CITY           "Taipei"
#define OWM_COUNTRY        "TW"
#define OWM_LANG           "zh_tw"
#define WEATHER_UPDATE_MS   (10 * 60 * 1000UL)  // 每 10 分鐘更新
```

### NTP 時間同步

```cpp
#define NTP_SERVER1        "pool.ntp.org"
#define NTP_SERVER2        "time.asia.apple.com"
#define TZ_OFFSET_SEC      (8 * 3600)    // UTC+8 台灣
```

### 密碼設定

```cpp
#define DEFAULT_PASSWORD   "1234"       // 開鎖密碼
#define ADMIN_PASSWORD     "9999"       // 管理員密碼
#define MAX_PASSWORD_LEN   8
#define MAX_FAIL_ATTEMPTS  5            // 連續失敗幾次觸發警報
```

### PIR 感測設定

```cpp
#define SLEEP_TIMEOUT_SEC   15     // 無人多久進入休眠（秒）
#define PIR_COOLDOWN_SEC    30     // 天氣播報冷卻時間（秒）
```

### 系統行為

```cpp
#define UNLOCK_DURATION_MS  5000    // 開鎖持續時間（毫秒）
#define FACE_SCAN_INTERVAL_MS 300 // 人臉掃描間隔
#define LOW_BATTERY_PCT    20      // 低電量警告
#define CRITICAL_BATTERY_PCT 10    // 嚴重低電量
```

### 功能開關

```cpp
#define FACE_RECOGNITION_EN   true   // 啟用本地人臉辨識
#define STRANGER_ALERT_EN     true   // 偵測陌生人時傳 Telegram 照片
#define WEATHER_NOTIFY_EN     true   // 開門時語音/顯示天氣提醒
#define FINGERPRINT_EN        true   // 啟用 AS608 指紋
```

### 人臉資料庫設定

```cpp
#define FACE_DB_DIR        "/faces"
#define MAX_FACE_COUNT     10        // 最多儲存幾張人臉
#define FACE_ENROLL_SAMPLES 5        // 登錄時連拍幾張
```

---

## 📦 燒錄指南

### 燒錄前準備

1. **確認 USB 連接**
   - Windows：裝置管理員 → 連接埠 → 確認 COM 編號
   - 確認使用**資料線**（非充電線）

2. **進入下載模式**
   - 按住 BOOT 按鈕
   - 按一下 RST 按鈕
   - 放開 BOOT 按鈕

### 燒錄步驟

```bash
# 1. 清理並編譯
pio run --target clean
pio run

# 2. 上傳主程式
pio run --target upload

# 3. 上傳 SPIFFS 檔案系統（首次燒錄必需）
pio run --target uploadfs

# 4. 開啟序列埠監視器
pio device monitor --port COM8 --baud 115200
```

### 分割表設定

本專案使用自訂分割表 `partitions_smartlock.csv`：

```csv
# Name,   Type, SubType, Offset,   Size,    Flags
nvs,      data, nvs,     0x9000,   0x5000,
otadata,  data, ota,     0xe000,   0x2000,
app0,     app,  ota_0,   0x10000,  0x500000,
spiffs,   data, spiffs,  0x510000, 0x2F0000,
```

| 分區 | 大小 | 用途 |
|------|------|------|
| nvs | 20KB | WiFi 設定等持久化資料 |
| otadata | 8KB | OTA 更新資料 |
| app0 | 5MB | 主程式儲存區 |
| spiffs | ~3MB | SPIFFS 檔案系統 |

---

## ❓ 常見問題

### 編譯相關

#### Q: 編譯失敗，找不到 esp-face 相關檔案？

**A**: 執行以下命令清理並重新編譯：
```bash
pio run --target clean
pio run
```

#### Q: 編譯時記憶體不足？

**A**: 
- 確認 `platformio.ini` 中設有 `-DBOARD_HAS_PSRAM`
- 減少 `fb_count`（鏡頭 frame buffer 數量）
- 確認分割表有足夠大的 app 分區

### 硬體相關

#### Q: OLED 顯示異常？

**A**: 
1. 確認 I2C 接線（SDA/SCL）
2. 檢查 OLED 地址是否為 `0x3C`
3. 確認 3.3V 供電正常
4. 使用 I2C Scanner 確認位址

#### Q: 鍵盤按鍵無回應？

**A**: 
1. 確認 PCF8574 地址是否正確（0x20）
2. 檢查 A0/A1/A2 焊點設定
3. 確認鍵盤排線順序是否正確
4. 檢查 KEY_MAP 是否與硬體匹配

#### Q: 指紋模組無回應？

**A**: 
1. 確認 AS608 使用 3.3V（不是 5V！）
2. 檢查 TX/RX 是否交叉連接
3. 確認 UART 鮑率為 57600
4. 測試：指紋感測器亮綠燈表示正常

#### Q: 人臉辨識準確率低？

**A**: 
- 增加登錄照片數量
- 登錄時拍攝不同角度
- 確保光線條件一致
- 考慮加裝紅外補光燈
- 調整鏡頭的 brightness/contrast 參數

#### Q: PIR 誤觸發？

**A**: 
- 調整安裝位置，遠離窗戶和熱源
- PIR 約需 30 秒預熱穩定
- 確認 VCC 為 3.3V
- 調整 HC-SR501 靈敏度旋鈕

### 網路相關

#### Q: WiFi 連線失敗？

**A**: 
- 確認 SSID 和密碼正確
- 檢查是否需要 Captive Portal 認證
- 確認路由器支援 2.4GHz（本專案不支援 5GHz）
- 增加 `WIFI_TIMEOUT_SEC`

#### Q: Telegram Bot 無法接收指令？

**A**: 
1. 確認 BOT_TOKEN 正確
2. 確認 CHAT_ID 正確
3. 執行刪除 Webhook：
   ```
   https://api.telegram.org/bot<TOKEN>/deleteWebhook?drop_pending_updates=true
   ```
4. 檢查 WiFi 是否正常連線

#### Q: 天氣 API 無法取得資料？

**A**: 
- 確認 OWM_API_KEY 有效
- 檢查城市名稱是否正確
- 確認 WiFi 網路可存取外部 API

### 系統運作

#### Q: 休眠時仍可遠端解鎖嗎？

**A**: 可以，WiFi 和 Telegram 在休眠期間持續運作

#### Q: 系統如何處理斷電？

**A**: 
- IP5306 UPS 模組自動切換到電池供電
- 密碼、人臉資料不會丟失（NVS/SPIFFS）
- 復電後自動重啟

#### Q: 如何備份人臉資料？

**A**: 
- 透過 Telegram `/face_list` 查看清單
- SPIFFS 資料位於 `/faces/` 目錄
- 可使用 `pio run --target uploadfs` 上傳預先準備的資料

---

## 📝 版本資訊

| 版本 | 日期 | 說明 |
|------|------|------|
| V3.0 | 2026/05/15 | PIR 人體感測器版，加入節能喚醒機制 |
| V2.0 | 2025/xx/xx | 多種解鎖方式整合 |
| V1.0 | 2025/xx/xx | 初始版本 |

### 最新功能 (V3.0)

- ✅ PIR 人體感測器喚醒（門外 + 門內）
- ✅ 休眠節能模式（功耗從 200mA 降至 ~70mA）
- ✅ 門內 PIR 觸發天氣語音播報
- ✅ WAV 音效播放支援
- ✅ 陌生人警報（自動拍照通知）
- ✅ 密碼 NVS 持久化儲存
- ✅ Telegram 訊息堆積清除機制

---

## 📄 授權與致謝

### 授權

本專案基於 MIT 授權條款開源，詳見 [LICENSE](LICENSE) 檔案。

### 致謝

本專案使用以下開源專案和函式庫：

| 專案 | 授權 | 用途 |
|------|------|------|
| [esp-face](https://github.com/espressif/esp-face) | Espressif | 本地人臉辨識框架（MTCNN + MobileFaceNet）|
| [esp32-camera](https://github.com/espressif/esp32-camera) | Espressif | OV2640 鏡頭驅動 |
| [Universal-Arduino-Telegram-Bot](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot) | MIT | Telegram Bot 通訊 |
| [Adafruit SSD1306](https://github.com/adafruit/Adafruit_SSD1306) | BSD | OLED 顯示器驅動 |
| [Adafruit GFX Library](https://github.com/adafruit/Adafruit-GFX-Library) | BSD | 圖形顯示核心 |
| [Adafruit Fingerprint Sensor Library](https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library) | MIT | AS608 指紋驅動 |
| [ArduinoJson](https://github.com/bblanchon/ArduinoJson) | MIT | JSON 解析 |
| [PubSubClient](https://github.com/knolleary/PubSubClient) | MIT | MQTT 客戶端 |
| [ESP8266Audio](https://github.com/earlephilhower/ESP8266Audio) | MIT | WAV 音頻播放 |

### 開發環境

- **IDE**: Visual Studio Code + PlatformIO IDE
- **Framework**: Arduino + ESP-IDF
- **Target Hardware**: Seeed XIAO ESP32-S3 Sense
- **Compiler**: espressif32 @ ~6.5.0

---

## 🔗 相關連結

- [XIAO ESP32-S3 Sense 官方頁面](https://www.seeedstudio.com/XIAO-ESP32S3-Sense-p-5676.html)
- [PlatformIO 官方文件](https://docs.platformio.org/)
- [ESP-IDF 程式設計指南](https://docs.espressif.com/projects/esp-idf/)
- [OpenWeatherMap API](https://openweathermap.org/api)
- [Telegram BotFather](https://t.me/BotFather)

---

<div align="center">

**Made with ❤️ for Smart Home Security**

**Version**: 3.0 | **Last Updated**: 2026/05/29

</div>
