# 🔐 XIAO ESP32 S3 Sense 智慧門鎖 完整教學（最終版）
### 從零開始的嵌入式系統專題 — 人臉辨識 · 指紋 · 密碼 · OLED · 電池備援 · 雲端通知 · 超聲波近接喚醒

> **適用對象**：完全沒有嵌入式開發經驗的初學者。本教學假設你只會基本的電腦操作，從硬體採購、接線、環境設定到每一行完整原始碼，一步一步帶你完成，並在過程中解釋每個元件和程式的原理。
>
> **包含**：40+ 個章節 · 完整原始碼 14 支程式檔 · 逐步測試清單 41 項（含超聲波 F 階段）· 常見問題排查 14 則

---

## 📋 目錄

**第一部分：認識這個專題**
- [1. 系統功能總覽](#第一章系統功能總覽)
- [2. 嵌入式系統基本概念](#第二章嵌入式系統基本概念)
- [3. 整體架構圖](#第三章整體架構圖)

**第二部分：硬體準備**
- [4. 完整硬體清單](#第四章完整硬體清單)（含 HC-SR04）
- [5. 各模組原理說明](#第五章各模組原理說明)
- [6. 完整接線指南](#第六章完整接線指南)（含分壓電路）

**第三部分：開發環境**
- [7. 安裝 PlatformIO](#第七章安裝-platformio)
- [8. 建立專案與設定](#第八章建立專案與設定)

**➕ 超聲波近接喚醒（新增功能）**
- [U1. 功能設計說明](#u1-功能設計說明)
- [U2. HC-SR04 接線](#u2-硬體hc-sr04-接線)
- [U3. GPIO 最終分配](#u3-gpio-腳位規劃最終完整版)
- [U4. ultrasonic.h](#u4-新增原始碼ultrasonich)
- [U5. config.h 新增項](#u5-修改configh新增設定項)
- [U6. main.cpp 完整更新版](#u6-修改maincpp完整更新版)
- [U7. 單獨測試程式](#u7-單獨測試程式)
- [U8~U10. 測試清單 / 調校 / Q&A](#u8-測試清單新增)

**第四部分：逐模組測試（Phase 1）**
- [9. I2C 掃描測試](#第九章i2c-掃描測試)
- [10. OLED 顯示器測試](#第十章oled-顯示器測試)
- [11. 4×4 鍵盤測試](#第十一章4×4-鍵盤測試)
- [12. AS608 指紋模組測試](#第十二章as608-指紋模組測試)
- [13. MAX98357 音頻測試](#第十三章max98357-音頻測試)
- [14. 繼電器與電磁鎖測試](#第十四章繼電器與電磁鎖測試)
- [15. 鏡頭與人臉辨識測試](#第十五章鏡頭與人臉辨識測試)
- [16. 電池電源測試](#第十六章電池電源測試)

**第五部分：雲端功能（Phase 2）**
- [17. WiFi 連線與 NTP 時間](#第十七章wifi-連線與-ntp-時間)
- [18. 天氣 API 整合](#第十八章天氣-api-整合)
- [19. Telegram Bot 設定](#第十九章telegram-bot-設定)

**第六部分：完整原始碼（Phase 3）**
- [20. 專案目錄結構](#第二十章專案目錄結構)
- [21. config.h — 設定檔](#第二十一章configh)
- [22. face_recognition.h — 人臉辨識](#第二十二章face_recognitionh)
- [23. face_database.h — 人臉資料庫](#第二十三章face_databaseh)
- [24. oled_ui.h — OLED 介面](#第二十四章oled_uih)
- [25. battery.h — 電池監控](#第二十五章batteryh)
- [26. keypad.h — 鍵盤掃描](#第二十六章keypadh)
- [27. fingerprint.h — 指紋模組](#第二十七章fingerprnth)
- [28. audio.h — 音頻播放](#第二十八章audioh)
- [29. relay.h — 繼電器](#第二十九章relayh)
- [30. wifi_mgr.h — WiFi 管理](#第三十章wifi_mgrh)
- [31. weather.h — 天氣查詢](#第三十一章weatherh)
- [32. telegram_bot.h — Telegram Bot](#第三十二章telegram_both)
- [33. main.cpp — 主程式](#第三十三章maincpp)

**第七部分：設定、測試與維護**
- [34. platformio.ini 與分割表](#第三十四章platformioini-與分割表)
- [35. 燒錄與完整測試清單](#第三十五章燒錄與完整測試清單)
- [36. 日常操作說明](#第三十六章日常操作說明)
- [37. 常見問題排查](#第三十七章常見問題排查)
- [38. 附錄：快速參考表](#第三十八章附錄快速參考表)

---

# 第一部分：認識這個專題

---

## 第一章：系統功能總覽

完成這個專題後，你的門鎖將具備以下所有功能：

| 功能 | 說明 | 硬體 |
|------|------|------|
| **人臉辨識開門** | 站到鏡頭前自動辨識，無需任何動作 | OV2640 鏡頭（內建）|
| **指紋辨識開門** | 按壓感測器即可解鎖 | AS608 指紋模組 |
| **密碼開門** | 輸入 4~8 位數字密碼 | 4×4 薄膜鍵盤 |
| **陌生人警報** | 偵測到未登錄人臉，自動拍照傳送到手機 | 鏡頭 + WiFi |
| **遠端開門** | 用 Telegram 傳送指令從任何地方開門 | WiFi + Telegram |
| **出門天氣提醒** | 開門後 OLED 顯示今天天氣，提醒帶傘 | OLED + 天氣 API |
| **語音提示** | 開鎖成功、失敗、警報都有對應聲音 | MAX98357 + 喇叭 |
| **人臉管理** | 新增、刪除授權人臉，最多 10 位 | SPIFFS 持久儲存 |
| **指紋管理** | 新增、刪除指紋，最多 127 筆 | AS608 內建 Flash |
| **電池備援** | 停電時自動切換電池，繼續正常運作 | TP4056 + 18650 |
| **電量顯示** | OLED 即時顯示電量百分比 | ADC + 分壓電路 |
| **狀態 LED** | 綠燈（解鎖成功）/ 紅燈（失敗/警報） | LED + PCF8574 |
| **OLED 顯示** | 即時顯示狀態、時間、天氣、操作提示 | SSD1306 128×64 |

---

## 第二章：嵌入式系統基本概念

> 💡 如果你已有嵌入式經驗，可跳過此章

### 什麼是嵌入式系統？

嵌入式系統是「嵌入在特定產品中的電腦系統」。與你電腦上的 Windows 不同，嵌入式系統通常只執行單一特定任務。門鎖控制器、洗碗機的控制板、汽車的引擎控制單元，都是嵌入式系統。

### 什麼是微控制器（MCU）？

微控制器是一種把 CPU（處理器）、RAM（記憶體）、Flash（儲存）、各種週邊介面全部整合在同一顆晶片上的裝置。我們用的 **ESP32-S3** 就是一款微控制器，它還內建 WiFi 和藍牙，非常適合物聯網專題。

```
你的電腦：
  CPU（Intel/AMD）+ RAM（DDR5）+ 硬碟（SSD）→ 三個獨立元件

ESP32-S3（微控制器）：
  CPU（Xtensa 雙核 240MHz）+ PSRAM（8MB）+ Flash（8MB）→ 全在一顆晶片上
```

### GPIO 是什麼？

GPIO（General Purpose Input/Output，通用輸入輸出腳位）是微控制器用來連接外部元件的腳位。每個 GPIO 可以被設定為：
- **輸出模式**：控制 LED 亮滅、驅動繼電器
- **輸入模式**：讀取按鈕狀態、偵測感測器信號

### 通訊協定是什麼？

不同元件之間傳遞資料需要共同的規則，這些規則就是「通訊協定」。本專題用到三種：

| 協定 | 線數 | 速度 | 本專題用途 |
|------|------|------|-----------|
| **UART** | 2（TX+RX）| 中等 | AS608 指紋模組 |
| **I2C** | 2（SDA+SCL）| 中等 | PCF8574 擴充、OLED |
| **I2S** | 3（BCLK+LRCLK+DATA）| 快 | MAX98357 數位音頻 |

#### UART（串列通訊）
最簡單的通訊方式：兩條線，一條傳送（TX）、一條接收（RX）。注意：**ESP32 的 TX 要接模組的 RX，不能接同側**。就像對講機，你說話對方要有耳朵聽。

#### I2C（兩線串列介面）
用兩條線（SDA 資料線 + SCL 時鐘線）連接多個裝置。每個裝置有唯一的 **7-bit 地址**，主機（ESP32）點名呼叫特定從機（PCF8574、OLED）。本專題把 OLED（0x3C）和兩個 PCF8574（0x20、0x21）全部掛在同一對 SDA/SCL 線上。

#### I2S（數位音頻介面）
專為音頻設計的協定，三條線傳輸高品質數位音訊：BCLK（位元時鐘）、LRCLK（左右聲道選擇）、DATA（音頻資料）。

### 什麼是 FreeRTOS？

ESP32 執行 FreeRTOS（即時作業系統），讓你可以用「任務（Task）」的概念同時執行多件事。本教學的音效播放就用了 FreeRTOS Task，讓音效在背景播放，不會阻塞人臉辨識的主迴圈。

---

## 第三章：整體架構圖

### 硬體連接架構

```
                    ┌─────────────────────────────────────────────────────┐
                    │              XIAO ESP32 S3 Sense                   │
                    │                                                     │
   ┌─────────┐      │  GPIO5(SDA) ─────────────────────────────────────► │
   │  OLED   │◄─────┤  GPIO6(SCL) ─────────────────────────────────────► │
   │ 0x3C    │      │                          ┌──────── I2C Bus ────┐   │
   └─────────┘      │                          │                     │   │
   ┌─────────┐      │                       ┌──▼──────┐  ┌──────────▼┐  │
   │PCF8574  │◄─────┤                       │PCF8574  │  │ PCF8574   │  │
   │鍵盤0x20 │      │                       │鍵盤0x20 │  │ LED 0x21  │  │
   └────┬────┘      │                       └──┬──────┘  └──────────┬┘  │
        │           │                          │(8腳)               │    │
   ┌────▼────┐      │                     ┌────▼────┐           ┌───▼──┐ │
   │ 4×4鍵盤 │      │                     │ 4×4鍵盤 │           │LED/  │ │
   └─────────┘      │                     └─────────┘           │蜂鳴器│ │
                    │                                           └──────┘ │
   ┌─────────┐      │  GPIO43(TX)──────────────────────────────────────► │
   │  AS608  │◄─────┤  GPIO44(RX)◄────────────────────────────────────── │
   │指紋模組 │      │                                                     │
   └─────────┘      │  GPIO2(BCLK)─────────────────────────────────────► │
                    │  GPIO3(LRCLK)────────────────────────────────────► │
   ┌─────────┐      │  GPIO4(DATA)─────────────────────────────────────► │
   │MAX98357 │◄─────┤                                                     │
   │ + 喇叭  │      │  GPIO1──────────────────────────────── 繼電器 IN   │
   └─────────┘      │                                             │       │
                    │  GPIO8(ADC)◄──────────────────────── 電池分壓     │
   ┌─────────┐      │  GPIO7(充電狀態)◄──────────────────── TP4056 CHRG │
   │TP4056   │◄─────┤                                                     │
   │+18650   │      │  內建鏡頭腳位（固定，不需接線）                     │
   │+IP5306  │      │  OV2640 Camera ←→ 鏡頭排線                        │
   └─────────┘      └─────────────────────────────────────────────────────┘
                                           │
                    ┌──────────────────────▼──────────────────────────┐
                    │               繼電器模組                          │
                    │  NC ───── 12V電源+ ───── 電磁鎖+ ───── 電磁鎖-  │
                    │  COM ──── 12V電源-（GND）                        │
                    └──────────────────────────────────────────────────┘
```

### 軟體系統架構

```
┌──────────────────────────────────────────────────────────────┐
│                        main.cpp                              │
│                                                              │
│  setup()                          loop()                     │
│  ├─ 初始化硬體                   ├─ handleIdle()            │
│  ├─ 初始化鏡頭/人臉辨識          │   ├─ OLED 待機畫面       │
│  ├─ 載入 SPIFFS 人臉資料庫       │   ├─ 掃描鍵盤            │
│  ├─ 初始化指紋                   │   ├─ 輪詢指紋感測器      │
│  ├─ 初始化音頻                   │   └─ 人臉辨識（300ms）   │
│  ├─ 連接 WiFi / 同步 NTP         ├─ handleUnlocked()       │
│  └─ 啟動 Telegram Bot            ├─ handleAlarm()          │
│                                  ├─ handleFaceMgmt()       │
│                                  ├─ handleTelegram()（3s） │
│                                  └─ checkBattery()（30s）  │
│                                                              │
│  狀態機：IDLE → 驗證 → UNLOCKED → IDLE                      │
│           IDLE → 連續失敗 → ALARM → IDLE                    │
│           IDLE → 管理密碼 → FACE_MGMT → IDLE                │
└──────────────────────────────────────────────────────────────┘
```

---

# 第二部分：硬體準備

---

## 第四章：完整硬體清單

### 4.1 核心元件

| 編號 | 元件名稱 | 規格 / 型號 | 數量 | 參考單價 | 採購提示 |
|------|---------|-----------|------|---------|---------|
| 1 | 主控板 | Seeed XIAO ESP32 S3 Sense | 1 | ~NT$600 | 注意要買 **Sense** 版（有鏡頭）|
| 2 | 指紋模組 | AS608（UART 介面）| 1 | ~NT$200 | 確認是 **UART** 版，非 SPI 版 |
| 3 | 鍵盤 | 4×4 薄膜矩陣鍵盤 | 1 | ~NT$60 | 有 8 條排線引出 |
| 4 | I/O 擴充 | PCF8574 模組（I2C）| 2 | ~NT$40/個 | 買帶有地址跳線的模組板 |
| 5 | OLED 顯示器 | SSD1306 128×64 I2C 模組 | 1 | ~NT$80 | I2C 介面版本（4 腳：VCC/GND/SDA/SCL）|
| 6 | 音頻放大器 | MAX98357A I2S 模組 | 1 | ~NT$150 | Adafruit 版或相容模組 |
| 7 | 喇叭 | 4Ω 3W 小喇叭 | 1 | ~NT$50 | 直徑 40~50mm 最佳 |
| 8 | 繼電器模組 | 5V 單路繼電器 | 1 | ~NT$50 | 選有光耦隔離的版本 |
| 9 | 電磁鎖 | 12V 電磁門鎖（NC 常閉型）| 1 | ~NT$300 | **NC 型**，斷電鎖門（安全）|
| 10 | 超聲波測距模組 | HC-SR04 | 1 | ~NT$40 | 5V 供電，近接喚醒用 |

### 4.2 電源元件

| 編號 | 元件名稱 | 規格 | 數量 | 採購提示 |
|------|---------|------|------|---------|
| 10 | 電磁鎖電源 | 12V 2A 變壓器 | 1 | 接頭確認與電磁鎖匹配 |
| 11 | 鋰電池充電模組 | TP4056（Type-C 版，帶保護板）| 1 | 選有過充過放保護的版本 |
| 12 | 鋰電池 | 18650 3.7V 2000mAh 以上 | 2 | 選正規品牌（Samsung/Panasonic）|
| 13 | UPS 模組 | IP5306（或 MT3608）| 1 | 支援「同時充放電（Pass-Through）」|
| 14 | DC-DC 降壓 | 12V→5V 2A（LM2596 模組）| 1 | 從 12V 電源取 5V 給系統 |

### 4.3 電子零件

| 編號 | 元件 | 規格 | 數量 | 用途 |
|------|------|------|------|------|
| 15 | 電阻 | 4.7kΩ（1/4W）| 4 | I2C 上拉電阻（SDA×2, SCL×2）|
| 16 | 電阻 | 100kΩ（1/4W）| 2 | 電池電壓分壓器 |
| 17 | 電阻 | 10kΩ（1/4W）| 1 | TP4056 CHRG 上拉 |
| 18 | 電阻 | 330Ω（1/4W）| 4 | LED 限流電阻 |
| 19 | 電阻 | 1kΩ（1/4W）| 1 | HC-SR04 ECHO 分壓上臂（5V→3.3V）|
| 20 | 電阻 | 2kΩ（1/4W）| 1 | HC-SR04 ECHO 分壓下臂 |
| 19 | 電容 | 100μF 電解電容（10V）| 2 | 電源去耦 |
| 20 | 電容 | 100nF 陶瓷電容 | 4 | ADC 去耦、I2C 去耦 |
| 21 | LED | 5mm 綠色 | 2 | 解鎖成功指示 |
| 22 | LED | 5mm 紅色 | 2 | 失敗/警報指示 |

### 4.4 工具與耗材

| 物品 | 用途 |
|------|------|
| 麵包板（830 孔）× 2 | 原型製作，免焊接 |
| 杜邦線（公-母）× 40 條 | 連接各模組 |
| 杜邦線（公-公）× 20 條 | 麵包板間連接 |
| USB-C 傳輸線 | 程式燒錄（注意要用**資料線**，非充電線）|
| 三用電表 | 量電壓、測通路（強烈建議）|
| 電路圖軟體（選用）| Fritzing（免費）畫接線圖 |

> 💡 **採購建議**：以上元件在蝦皮、露天、Arduino 台灣代理商（如 Maker.com.tw）均可購得。建議多買備用（尤其 PCF8574 和 LED），因為嵌入式學習過程難免會有元件損壞。

---

## 第五章：各模組原理說明

### 5.1 XIAO ESP32 S3 Sense 腳位圖

```
           USB-C
    ┌──────[  ]──────┐
  D0│GPIO1           │GPIO43 D6
  D1│GPIO2           │GPIO44 D7
  D2│GPIO3           │GPIO45 D8
  D3│GPIO4           │GPIO48 D9
  D4│GPIO5(SDA)      │3.3V
  D5│GPIO6(SCL)      │GND
  D6│GPIO43(TX)      │5V
  D7│GPIO44(RX)      │GND
    │                │
    │   [BOOT] [RST] │
    └────────────────┘
         底部有鏡頭排線連接器
```

> ⚠️ **注意**：GPIO35~42 是鏡頭占用的腳位，不能用於其他用途。

### 5.2 PCF8574 I/O 擴充

**為什麼需要 PCF8574？**

4×4 鍵盤需要 8 個 GPIO（4 Row + 4 Col）。但 XIAO ESP32 S3 的可用 GPIO 在接完鏡頭後已經很少，所以我們用 PCF8574 透過 I2C（只用 2 條線）擴充 8 個 GPIO。兩個 PCF8574 只用 SDA 和 SCL 共 2 條線就擴充了 16 個 GPIO。

**地址設定**：PCF8574 背面有 A0、A1、A2 三個焊點，決定 I2C 地址：

```
A2  A1  A0   地址
 0   0   0   0x20  ← 鍵盤用（三個都接 GND）
 0   0   1   0x21  ← LED/狀態用（A0 接 VCC，其他接 GND）
```

### 5.3 AS608 指紋模組工作原理

```
指紋登錄流程：
  按壓手指 → 光學感測器拍攝指紋影像
           → 內部 DSP 提取特徵點（端點、分叉點）
           → 生成特徵模板（CharBuffer）
  第二次按壓 → 再生成一個模板
           → 比對兩次模板 → 合成最終模板
           → 儲存到 Flash（ID 1~127）

指紋驗證流程：
  按壓手指 → 生成特徵模板
           → 與所有已儲存模板比對
           → 回傳最高匹配度的 ID 和信心分數（0~100）
```

### 5.4 人臉辨識：MTCNN + MobileFaceNet

本專題使用 Espressif 提供的 **esp-face** 框架，在 ESP32 S3 上本地執行兩個神經網路：

```
第一階段：HumanFaceDetectMSR01（MTCNN-PNet）
  輸入：RGB565 240×240 影像
  功能：快速掃描影像，找出可能有人臉的區域
  輸出：候選邊框清單
  速度：~50ms

第二階段：HumanFaceDetectMNP01（MTCNN-RNet+ONet）
  輸入：候選邊框
  功能：精確定位人臉，找出5個關鍵點（左眼、右眼、鼻子、嘴角）
  輸出：精確邊框 + 5個關鍵點坐標
  速度：~150ms

辨識：FaceRecognition112V1S16（MobileFaceNet）
  輸入：裁切並對齊的 112×112 人臉影像
  功能：提取 512 維特徵向量，與資料庫比對餘弦相似度
  輸出：最匹配的人員 ID（或 -1 = 陌生人）
  速度：~300ms
```

### 5.5 電池備援電路

```
正常供電（USB 接入時）：
USB → TP4056 → [對電池充電] → IP5306 → 5V → 系統

斷電後（USB 拔除時）：
18650 電池 → IP5306 → 5V → 系統（自動無縫切換，<20ms）

電量監測：
電池正極(3.7~4.2V) → 100kΩ → [ADC 點] → 100kΩ → GND
                                    ↓
                               GPIO8(ADC)
ADC 讀到電壓 = 電池電壓 / 2（最高 2.1V，在 3.3V 量程內）
```

---

## 第六章：完整接線指南

> ⚠️ **接線前的安全提醒**：
> 1. 所有接線在 ESP32 **未通電**的情況下進行
> 2. 電磁鎖使用 12V，絕對不能接到 ESP32 的 5V 腳位
> 3. 接完一個模組就測試一個，不要一次接完再測

### 6.1 I2C 匯流排（共用 SDA/SCL）

所有 I2C 裝置並聯在同一對 SDA/SCL 線上：

```
ESP32 D4 (GPIO5, SDA) ──┬──── PCF8574 #1 SDA
                         ├──── PCF8574 #2 SDA
                         └──── OLED SDA

ESP32 D5 (GPIO6, SCL) ──┬──── PCF8574 #1 SCL
                         ├──── PCF8574 #2 SCL
                         └──── OLED SCL

3.3V ──── 4.7kΩ ──── SDA（上拉電阻，接麵包板的 SDA 公共線）
3.3V ──── 4.7kΩ ──── SCL（上拉電阻）
```

> 💡 在麵包板上找兩條橫向的電源軌，一條接 SDA、一條接 SCL，
> 所有模組的 SDA/SCL 都插到這兩條軌上，省去重複佈線。

### 6.2 PCF8574 #1（地址 0x20）— 鍵盤

```
PCF8574 #1 模組接線：
  VCC → 3.3V
  GND → GND
  SDA → I2C SDA 公共線
  SCL → I2C SCL 公共線
  A0  → GND（地址位元 0）
  A1  → GND（地址位元 1）
  A2  → GND（地址位元 2）
  
PCF8574 #1 腳位 → 4×4 鍵盤排線：
  P0 → 鍵盤第 1 條線（Row 1）
  P1 → 鍵盤第 2 條線（Row 2）
  P2 → 鍵盤第 3 條線（Row 3）
  P3 → 鍵盤第 4 條線（Row 4）
  P4 → 鍵盤第 5 條線（Col 1）
  P5 → 鍵盤第 6 條線（Col 2）
  P6 → 鍵盤第 7 條線（Col 3）
  P7 → 鍵盤第 8 條線（Col 4）

4×4 鍵盤按鍵排列：
      Col1 Col2 Col3 Col4
Row1 [ 1 ] [ 2 ] [ 3 ] [ A ]
Row2 [ 4 ] [ 5 ] [ 6 ] [ B ]
Row3 [ 7 ] [ 8 ] [ 9 ] [ C ]
Row4 [ * ] [ 0 ] [ # ] [ D ]
```

> 💡 **如何確認鍵盤排線順序**：薄膜鍵盤的排線通常有編號 1~8，對應方向看鍵盤正面，排線在右側時，由上到下是 R1, R2, R3, R4, C1, C2, C3, C4。若順序不對，掃描到的按鍵字元會對不上，屆時在程式的 keyMap 陣列中調整即可。

### 6.3 PCF8574 #2（地址 0x21）— 狀態指示

```
PCF8574 #2 模組接線：
  VCC → 3.3V
  GND → GND
  SDA → I2C SDA 公共線
  SCL → I2C SCL 公共線
  A0  → 3.3V（地址位元 0 = 1）
  A1  → GND
  A2  → GND

PCF8574 #2 腳位 → LED：
  P0 → 330Ω 電阻 → 綠色 LED 正極 → LED 負極 → GND
  P1 → 330Ω 電阻 → 紅色 LED 正極 → LED 負極 → GND
  P2 → 330Ω 電阻 → 蜂鳴器正極 → 蜂鳴器負極 → GND（選配）
  P3~P7 → 不接（預留擴充）

注意：PCF8574 輸出 LOW 時 LED 亮（反邏輯），
      程式中 write(0, LOW) = 綠燈亮
```

### 6.4 OLED 顯示器

```
SSD1306 OLED 接線：
  VCC → 3.3V
  GND → GND
  SDA → I2C SDA 公共線
  SCL → I2C SCL 公共線
  
（無需額外設定，I2C 地址固定為 0x3C）
```

### 6.5 AS608 指紋模組

```
AS608 接線（注意 TX/RX 交叉）：
  VCC   → 3.3V（重要：AS608 工作電壓 3.3V！）
  GND   → GND
  TXD   → ESP32 D7（GPIO44，這是 ESP32 的 RX）
  RXD   → ESP32 D6（GPIO43，這是 ESP32 的 TX）
  TOUCH → 不接（有些模組無此腳位）
  IRQ   → 不接（使用軟體輪詢模式）
```

### 6.6 MAX98357A 音頻放大器

```
MAX98357A 接線：
  VIN  → 5V（注意：需要 5V 才有足夠音量）
  GND  → GND
  BCLK → ESP32 D1（GPIO2）
  LRC  → ESP32 D2（GPIO3）
  DIN  → ESP32 D3（GPIO4）
  GAIN → 不接（預設 9dB 增益，可接 GND 為 12dB）
  SD   → 不接（預設啟用輸出）

喇叭接線：
  MAX98357A 的 "+" 端 → 喇叭正極
  MAX98357A 的 "-" 端 → 喇叭負極
```

### 6.7 繼電器模組與電磁鎖

```
繼電器模組接線（控制側）：
  VCC → 5V（繼電器線圈需要 5V）
  GND → GND（注意：與 ESP32 共地）
  IN  → ESP32 D0（GPIO1）

繼電器模組接線（電源側，12V 高壓側）：
  COM → 電磁鎖正極
  NC  → 12V 電源正極
  NO  → 不接

12V 電源：
  12V+ → 繼電器 NC
  12V- → 電磁鎖負極

邏輯說明：
  GPIO1 = HIGH → 繼電器不動作 → NC連通COM → 電磁鎖通電 → 門鎖緊 ✓
  GPIO1 = LOW  → 繼電器動作   → NC斷開    → 電磁鎖斷電 → 門打開 ✓
  停電時        → GPIO=LOW(浮接) → 繼電器不動作 → 電磁鎖通電 → 門鎖緊 ✓（安全）
```

> ⚠️ **12V 電源安全注意事項**：
> - 確認 12V 電源的 GND 與 ESP32 的 GND 相連（共地），否則繼電器控制訊號會不正常
> - 12V 接線用稍粗的電線（24AWG 以上）
> - 電磁鎖有線圈，斷電瞬間會產生反向電壓（反電動勢），建議在電磁鎖並聯一個 1N4007 二極管（正極接電磁鎖正極，負極接 12V+）保護電路

### 6.8 電池備援電路

```
TP4056 模組接線：
  IN+  → USB 5V 正極（或 DC-DC 降壓模組的 5V 輸出）
  IN-  → GND
  B+   → 18650 電池正極（若兩顆並聯則兩顆正極都接這裡）
  B-   → 18650 電池負極（GND）
  OUT+ → IP5306 BAT 腳位
  OUT- → GND

IP5306 模組接線：
  VIN+ → 5V（主電源 USB 5V 或 DC-DC）
  VIN- → GND
  BAT+ → TP4056 OUT+ 或電池正極
  BAT- → GND
  VOUT+ → ESP32 的 5V 腳位（系統電源）
  VOUT- → GND

電池電壓監測（分壓電路）：
  電池正極(3.7~4.2V) ──┬── 100kΩ ──┬── 100kΩ ── GND
                        │           │
                        │       ESP32 D9 (GPIO8) ADC 輸入
                    （電池正極）  （分壓中點，最高 ~2.1V）
  
  在 ADC 輸入腳位和 GND 之間加 100nF 電容（濾波）

充電狀態偵測：
  TP4056 CHRG 腳 ──┬── 10kΩ ── 3.3V（上拉）
                   └── ESP32 D8 (GPIO7)
  CHRG = LOW → 正在充電
  CHRG = HIGH → 充飽或未接充電電源
```

---

# 第三部分：開發環境

---

## 第七章：安裝 PlatformIO

本專題使用 **PlatformIO** 而非 Arduino IDE，原因是人臉辨識框架（esp-face）的 include 路徑設定在 PlatformIO 中更為可靠。別擔心，程式碼寫法與 Arduino 完全相同。

### 7.1 安裝步驟

**Step 1：安裝 Visual Studio Code**
1. 前往 https://code.visualstudio.com 下載對應你作業系統的版本
2. 安裝完成後開啟 VS Code

**Step 2：安裝 PlatformIO 擴充**
1. 點擊左側欄的「Extensions」圖示（四個方塊的圖案，快捷鍵 `Ctrl+Shift+X`）
2. 在搜尋框輸入 `PlatformIO IDE`
3. 找到 "PlatformIO IDE" by PlatformIO，點擊 **Install**
4. 安裝過程中 VS Code 會自動下載 Python 和 PlatformIO Core，需要 5~15 分鐘
5. 安裝完成後，VS Code 底部狀態列會出現 PlatformIO 的圖示（小螞蟻 🐜）
6. **重新啟動 VS Code**

### 7.2 連接 XIAO ESP32 S3

1. 用 USB-C 線連接 XIAO ESP32 S3 和電腦
2. **Windows**：開啟裝置管理員，應在「連接埠」下看到新的 COM 裝置（例如 COM8）
   **Mac**：在終端機執行 `ls /dev/tty.*`，應看到 `/dev/tty.usbmodem*`
   **Linux**：`ls /dev/ttyACM*`，應看到 `/dev/ttyACM0`
3. 若沒有出現：安裝 CH340 或 CP2102 驅動程式（視你的 USB 晶片型號）

> 💡 **XIAO ESP32 S3 Sense 使用原生 USB**，通常不需要額外驅動程式。若仍無法識別，嘗試按住開發板上的 **Boot 按鈕** 後再插入 USB。

---

## 第八章：建立專案與設定

### 8.1 建立新專案

1. 點擊 VS Code 左側的 PlatformIO 圖示（🐜）
2. 選擇 **"New Project"**
3. 填入以下設定：
   - **Name**: `SmartLock`
   - **Board**: 輸入 `XIAO`，選擇 `Seeed XIAO ESP32S3`
   - **Framework**: `Arduino`
   - **Location**: 選擇你想存放的資料夾
4. 點擊 **Finish**，等待 PlatformIO 下載工具鏈（第一次約需 5~10 分鐘）

### 8.2 設定 platformio.ini

開啟專案根目錄的 `platformio.ini`，**完全取代**為以下內容（詳細說明見第 34 章）：

```ini
[env:seeed_xiao_esp32s3]
platform  = espressif32 @ ~6.5.0
board     = seeed_xiao_esp32s3
framework = arduino

monitor_speed = 115200
upload_speed  = 921600

board_build.partitions          = partitions_smartlock.csv
board_build.arduino.memory_type = qio_opi

build_flags =
    -DBOARD_HAS_PSRAM
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DCORE_DEBUG_LEVEL=0
    -mfix-esp32-psram-cache-issue
    -I${platformio.packages_dir}/framework-arduinoespressif32/tools/sdk/esp32s3/include/esp-face/include/typedef
    -I${platformio.packages_dir}/framework-arduinoespressif32/tools/sdk/esp32s3/include/esp-face/include/math
    -I${platformio.packages_dir}/framework-arduinoespressif32/tools/sdk/esp32s3/include/esp-face/include/image
    -I${platformio.packages_dir}/framework-arduinoespressif32/tools/sdk/esp32s3/include/esp-face/include/model_zoo
    -I${platformio.packages_dir}/framework-arduinoespressif32/tools/sdk/esp32s3/include/esp-face/include/face_detection
    -I${platformio.packages_dir}/framework-arduinoespressif32/tools/sdk/esp32s3/include/esp-face/include/face_recognition

lib_deps =
    espressif/esp32-camera @ ^2.0.4
    adafruit/Adafruit SSD1306 @ ^2.5.7
    adafruit/Adafruit GFX Library @ ^1.11.9
    adafruit/Adafruit Fingerprint Sensor Library @ ^2.1.0
    renzo-mischianti/PCF8574 @ ^2.3.6
    bblanchon/ArduinoJson @ ^6.21.3
    knolleary/PubSubClient @ ^2.8.0
    schreibfaul1/ESP32-audioI2S @ ^2.0.7
    witnessmenow/Universal-Arduino-Telegram-Bot @ ^1.3.0
```

### 8.3 建立 partitions_smartlock.csv

在**專案根目錄**（與 `platformio.ini` 同一層）建立此檔案：

```csv
# Name,   Type, SubType,  Offset,   Size,    Flags
nvs,      data, nvs,      0x9000,   0x5000,
otadata,  data, ota,      0xe000,   0x2000,
app0,     app,  ota_0,    0x10000,  0x300000,
app1,     app,  ota_1,    0x310000, 0x300000,
spiffs,   data, spiffs,   0x610000, 0x1F0000,
```

### 8.4 驗證環境可以編譯

在 `src/main.cpp` 貼入以下內容並嘗試編譯（`Ctrl+Alt+B`）：

```cpp
#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.printf("CPU: %d MHz\n", getCpuFrequencyMhz());
    Serial.printf("PSRAM: %s, size: %d MB\n",
                  psramFound() ? "Found" : "Not Found",
                  ESP.getPsramSize() / 1024 / 1024);
    Serial.printf("Flash: %d MB\n", ESP.getFlashChipSize() / 1024 / 1024);
    Serial.println("環境設定正確！");
}

void loop() {}
```

點擊底部工具列的 **Upload** 箭頭（→）上傳，然後點 **Serial Monitor** 圖示開啟序列埠監視器。

**預期輸出**：
```
CPU: 240 MHz
PSRAM: Found, size: 8 MB
Flash: 8 MB
環境設定正確！
```

若看到以上輸出，代表開發環境完全正確，可以進入下一步。

---

# ➕ 新增功能：超聲波感測器近接喚醒

> 本章節為獨立擴充，直接整合進完整教學。新增 HC-SR04 超聲波模組，當偵測到有人靠近（≤ 80cm）時才啟動人臉辨識、指紋掃描等耗電功能；無人時系統進入休眠，OLED 關閉、鏡頭暫停，大幅降低功耗並延長電池壽命。

---

## 目錄

- [U1. 功能設計說明](#u1-功能設計說明)
- [U2. 硬體：HC-SR04 接線](#u2-硬體hc-sr04-接線)
- [U3. GPIO 腳位規劃（最終完整版）](#u3-gpio-腳位規劃最終完整版)
- [U4. 新增原始碼：ultrasonic.h](#u4-新增原始碼ultrasonich)
- [U5. 修改：config.h（新增設定項）](#u5-修改configh新增設定項)
- [U6. 修改：main.cpp（完整更新版）](#u6-修改maincpp完整更新版)
- [U7. 單獨測試程式](#u7-單獨測試程式)
- [U8. 測試清單（新增）](#u8-測試清單新增)
- [U9. 調校建議](#u9-調校建議)
- [U10. 常見問題](#u10-常見問題)

---

## U1. 功能設計說明

### 系統行為

```
┌──────────────────────────────────────────────────────────────────┐
│                      近接喚醒狀態機                                │
│                                                                    │
│   ┌─────────────────────────────────────────────────────────┐    │
│   │                   STATE_SLEEP（休眠）                    │    │
│   │  • HC-SR04 每 200ms 量測一次距離                         │    │
│   │  • OLED 關閉（省電）                                     │    │
│   │  • 鏡頭暫停（不取 frame buffer）                         │    │
│   │  • 指紋感測器停止輪詢                                    │    │
│   │  • WiFi + Telegram 維持（遠端開鎖仍可用）                │    │
│   │  • 功耗約 80mA（vs 正常 200mA）                          │    │
│   └────────────────────┬────────────────────────────────────┘    │
│                         │ 距離 ≤ 80cm                              │
│                         ▼                                          │
│   ┌─────────────────────────────────────────────────────────┐    │
│   │                   喚醒過渡（~0.5秒）                     │    │
│   │  • OLED 亮起，顯示「有人靠近」動畫                       │    │
│   │  • 播放短暫喚醒提示音                                    │    │
│   │  • 鏡頭重新啟動                                         │    │
│   └────────────────────┬────────────────────────────────────┘    │
│                         │                                          │
│                         ▼                                          │
│   ┌─────────────────────────────────────────────────────────┐    │
│   │                   STATE_IDLE（運作中）                   │    │
│   │  • 全功能運作（人臉 / 指紋 / 鍵盤 / OLED / Telegram）   │    │
│   │  • HC-SR04 每 500ms 量測（持續監控是否離開）             │    │
│   │  • 距離 > 80cm 且 15 秒無任何輸入 → 返回 SLEEP          │    │
│   └─────────────────────────────────────────────────────────┘    │
│                                                                    │
│   注意：STATE_UNLOCKED / STATE_ALARM / STATE_FACE_MGMT            │
│         期間不會進入 SLEEP（避免操作中斷）                          │
└──────────────────────────────────────────────────────────────────┘
```

### 為什麼 TRIG 用 PCF8574？

XIAO ESP32 S3 的 11 個外部 GPIO 在完整接線後只剩 **GPIO9（D10）** 一個空腳位。HC-SR04 需要 TRIG + ECHO 兩條線：

- **ECHO（時序關鍵）**：必須是直接 GPIO → 使用 **D10（GPIO9）**
- **TRIG（時序寬鬆）**：HC-SR04 只需要 ≥10μs 的脈衝，PCF8574 的 I2C 操作約 200μs，完全符合 → 使用 **PCF8574 #2 P3**

### 電壓轉換（重要！）

HC-SR04 的 ECHO 腳位輸出是 **5V**，但 ESP32 GPIO 的最大輸入電壓是 **3.3V**（超過會損壞）。必須用電阻分壓器降壓：

```
HC-SR04 ECHO (5V) ──── 1kΩ ──── D10 (GPIO9, 3.3V max)
                                      │
                                    2kΩ
                                      │
                                    GND

分壓輸出 = 5V × 2kΩ/(1kΩ+2kΩ) = 3.33V ✓
```

---

## U2. 硬體：HC-SR04 接線

### 新增元件

| 元件 | 規格 | 數量 | 備注 |
|------|------|------|------|
| 超聲波測距模組 | HC-SR04 | 1 | 5V 供電版本 |
| 電阻 | 1kΩ（1/4W）| 1 | ECHO 分壓上臂 |
| 電阻 | 2kΩ（1/4W）| 1 | ECHO 分壓下臂 |

> 💡 沒有 2kΩ？用兩個 1kΩ 串聯即可（總阻值 = 2kΩ）。

### 完整接線

```
HC-SR04 模組
┌─────────────┐
│  VCC        │──────────────────── 5V
│  GND        │──────────────────── GND
│  TRIG       │──────────────────── PCF8574 #2 P3
│  ECHO       │──── 1kΩ ──┬──────── D10 (GPIO9)
└─────────────┘           │
                         2kΩ
                           │
                          GND

安裝位置建議：
  門框上方或門旁牆面
  朝向走廊/門外方向
  距地面 1.2~1.5m 高度
  感測角度 15°，偵測範圍 2cm~400cm
```

### 安裝角度示意

```
 走廊/門外

    人→  ◉  ←  HC-SR04 掃描扇形
         |     (15° 角，偵測 80cm 內)
    ═════╪═════  門
         |
    室內
```

---

## U3. GPIO 腳位規劃（最終完整版）

加入 HC-SR04 後的完整腳位分配：

| 功能 | XIAO 腳位 | GPIO | 連接 |
|------|----------|------|------|
| I2C SDA | D4 | GPIO5 | OLED / PCF8574 #1 / #2 |
| I2C SCL | D5 | GPIO6 | OLED / PCF8574 #1 / #2 |
| AS608 TX→ | D6 | GPIO43 | AS608 RXD |
| AS608 RX← | D7 | GPIO44 | AS608 TXD |
| I2S BCLK | D1 | GPIO2 | MAX98357 BCLK |
| I2S LRCLK | D2 | GPIO3 | MAX98357 LRC |
| I2S DATA | D3 | GPIO4 | MAX98357 DIN |
| 繼電器 | D0 | GPIO1 | 繼電器 IN |
| 電池 ADC | D9 | GPIO8 | 分壓中點 |
| 充電狀態 | D8 | GPIO7 | TP4056 CHRG |
| **HC-SR04 ECHO** | **D10** | **GPIO9** | **ECHO（經分壓）** |

**PCF8574 #2（0x21）最終腳位分配**：

| 腳位 | 功能 |
|------|------|
| P0 | 綠色 LED |
| P1 | 紅色 LED |
| P2 | 蜂鳴器（選配）|
| **P3** | **HC-SR04 TRIG** |
| P4~P7 | 預留 |

---

## U4. 新增原始碼：ultrasonic.h

在 `src/` 目錄下建立此新檔案：

```cpp
// src/ultrasonic.h
// HC-SR04 超聲波近接偵測模組
// TRIG：PCF8574 #2 P3（I2C 輸出）
// ECHO：GPIO9 (D10)（直接 GPIO，經 1kΩ/2kΩ 分壓）
#pragma once

#include <Arduino.h>
#include <PCF8574.h>
#include "config.h"

class UltrasonicSensor {
public:
    // ── 建構子：傳入 PCF8574 #2 的參考 ──────────────
    UltrasonicSensor(PCF8574& statusPcf) : _pcf(statusPcf) {}

    // ── 初始化 ────────────────────────────────────────
    void begin() {
        pinMode(US_ECHO_PIN, INPUT);
        _pcf.write(US_TRIG_P, LOW);  // TRIG 預設 LOW
        Serial.println("✅ HC-SR04 初始化完成");
        Serial.printf("   TRIG = PCF8574 #2 P%d\n", US_TRIG_P);
        Serial.printf("   ECHO = GPIO%d (D10)\n", US_ECHO_PIN);
    }

    // ── 量測距離（公分）────────────────────────────────
    // 回傳值：距離 cm（2~400cm 有效）
    //         -1 = 超時（無回波，表示距離太遠或無障礙物）
    float measureCm() {
        // 發送觸發脈衝（透過 PCF8574 I2C）
        // PCF8574 I2C write 約 150μs，兩次合計約 300μs
        // HC-SR04 只需 ≥10μs，300μs 完全符合
        _pcf.write(US_TRIG_P, HIGH);   // TRIG HIGH
        _pcf.write(US_TRIG_P, LOW);    // TRIG LOW（約 300μs 後）

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
    PCF8574& _pcf;
    float    _lastDistance = 999.0f;
};
```

---

## U5. 修改：config.h（新增設定項）

在 `config.h` 的「硬體腳位」區塊新增以下內容：

```cpp
// ════════════════════════════════════════
//  HC-SR04 超聲波近接感測器
// ════════════════════════════════════════
#define US_ECHO_PIN         9      // GPIO9 = D10（直接 GPIO）
#define US_TRIG_P           3      // PCF8574 #2 的 P3（I2C 輸出）

// ════════════════════════════════════════
//  近接喚醒行為設定
// ════════════════════════════════════════
#define WAKE_DISTANCE_CM    80     // 距離 ≤ 80cm 時喚醒系統（公分）
#define SLEEP_TIMEOUT_SEC   15     // IDLE 狀態下，幾秒無人就進入休眠
#define US_SLEEP_INTERVAL   200    // 休眠狀態下的量測間隔（毫秒）
#define US_IDLE_INTERVAL    500    // 待機狀態下的量測間隔（毫秒）
```

同時在 `SystemState` enum 新增 `STATE_SLEEP`：

```cpp
// 將 config.h 中的 SystemState 改為：
enum SystemState {
    STATE_SLEEP,       // ← 新增：休眠，等待有人靠近
    STATE_IDLE,
    STATE_UNLOCKED,
    STATE_ALARM,
    STATE_FACE_MGMT
};
```

---

## U6. 修改：main.cpp（完整更新版）

以下是加入超聲波功能後的 **完整 main.cpp**，取代原本的版本：

```cpp
// src/main.cpp  —  智慧門鎖主程式（含超聲波近接喚醒）
#include <Arduino.h>
#include <Wire.h>
#include <PCF8574.h>
#include "config.h"
#include "ultrasonic.h"       // ← 新增
#include "face_recognition.h"
#include "face_database.h"
#include "oled_ui.h"
#include "battery.h"
#include "keypad.h"
#include "fingerprint.h"
#include "audio.h"
#include "relay.h"
#include "wifi_mgr.h"
#include "weather.h"
#include "telegram_bot.h"

// ════ 全域物件 ════
FaceRecognitionSystem faceSystem;
FaceDatabase          faceDB;
OledUI                ui;
BatteryMonitor        battery;

PCF8574 keypadPCF(PCF_KEYPAD_ADDR, I2C_SDA_PIN, I2C_SCL_PIN);
PCF8574 statusPCF(PCF_STATUS_ADDR, I2C_SDA_PIN, I2C_SCL_PIN);
UltrasonicSensor  sonar(statusPCF);   // ← 新增，傳入 PCF8574 #2

HardwareSerial       fpSerial(1);
Adafruit_Fingerprint finger(&fpSerial);

// ════ 執行時狀態 ════
SystemState  currentState    = STATE_SLEEP;   // ← 初始為 SLEEP
String       currentPassword = DEFAULT_PASSWORD;
int          failCount       = 0;
WeatherInfo  weatherCache;

// ════ 時間戳記 ════
unsigned long lastWeatherUpdate  = 0;
unsigned long lastBotPoll        = 0;
unsigned long lastBattCheck      = 0;
unsigned long lastOledUpdate     = 0;
unsigned long lastSonarCheck     = 0;  // ← 新增
unsigned long lastActivityTime   = 0;  // ← 新增：最後一次有人活動的時間

// ════ 鍵盤輸入緩衝 ════
String inputBuf = "";

// ════════════════════════════════════════════
//  工具函式
// ════════════════════════════════════════════

void setLED(bool green, bool red) {
    statusPCF.write(LED_GREEN_P, green ? LOW : HIGH);
    statusPCF.write(LED_RED_P,   red   ? LOW : HIGH);
}

// 更新「最後活動時間」（任何按鍵、指紋、人臉都呼叫此函式）
void markActivity() {
    lastActivityTime = millis();
}

// ════════════════════════════════════════════
//  解鎖 / 失敗 / 警報
// ════════════════════════════════════════════

void successUnlock(const String& name, camera_fb_t* photo = nullptr) {
    Serial.printf("🔓 解鎖 [%s]\n", name.c_str());
    failCount    = 0;
    currentState = STATE_UNLOCKED;
    inputBuf     = "";
    markActivity();

    unlockDoor(UNLOCK_DURATION_MS);
    playSoundAsync(SOUND_UNLOCK);
    setLED(true, false);

    String weather = WEATHER_NOTIFY_EN ? getWeatherMessage(weatherCache) : "";
    ui.showUnlocked(name, weather, UNLOCK_DURATION_MS / 1000);

    sendTelegramMessage("✅ 解鎖：" + name + "\n" +
                        getCurrentDateTime() + "\n" + weather);
}

void failedAttempt() {
    failCount++;
    markActivity();
    playSoundAsync(SOUND_DENY);
    setLED(false, true);
    ui.showDenied(failCount, MAX_FAIL_ATTEMPTS);
    delay(1200);
    setLED(false, false);

    if (failCount >= MAX_FAIL_ATTEMPTS) {
        currentState = STATE_ALARM;
        sendTelegramMessage("🚨 警報！連續失敗 " +
                            String(MAX_FAIL_ATTEMPTS) + " 次\n" +
                            getCurrentDateTime());
    }
}

// ════════════════════════════════════════════
//  鍵盤
// ════════════════════════════════════════════

void handleKeyInput(char key) {
    markActivity();
    playSoundAsync(SOUND_BEEP);

    if (key == '*') {
        inputBuf = "";
        ui.showPasswordInput(0);
        return;
    }
    if (key == '#') {
        if (inputBuf == currentPassword)       successUnlock("Password");
        else                                   failedAttempt();
        inputBuf = "";
        return;
    }
    if (key == 'A') {
        if (inputBuf == ADMIN_PASSWORD) {
            currentState = STATE_FACE_MGMT;
        } else {
            ui.showMessage("Access Denied", "Wrong admin pwd");
            delay(1500);
        }
        inputBuf = "";
        return;
    }
    if (isDigit(key) && inputBuf.length() < (unsigned)MAX_PASSWORD_LEN) {
        inputBuf += key;
        ui.showPasswordInput(inputBuf.length());
    }
}

// ════════════════════════════════════════════
//  STATE_SLEEP：休眠，等待有人靠近
// ════════════════════════════════════════════

void handleSleep() {
    // 用低頻率量測，節省功耗
    if (millis() - lastSonarCheck < US_SLEEP_INTERVAL) return;
    lastSonarCheck = millis();

    if (sonar.isPersonNearby(WAKE_DISTANCE_CM)) {
        float d = sonar.getLastDistance();
        Serial.printf("👤 偵測到有人靠近（%.1f cm），喚醒系統\n", d);

        // ── 喚醒序列 ──
        // 1. 開啟 OLED
        ui.showMessage("Someone nearby...", String((int)d) + " cm detected");

        // 2. 播放喚醒提示音（短促友善音）
        playSoundAsync(SOUND_BEEP);

        // 3. 切換狀態
        currentState   = STATE_IDLE;
        lastActivityTime = millis();

        Serial.println("✅ 系統已喚醒");
    }
    // 若無人，維持 OLED 關閉狀態
}

// ════════════════════════════════════════════
//  STATE_IDLE：正常待機，全功能運作
// ════════════════════════════════════════════

void handleIdle() {
    // ── 無人自動進入休眠 ──────────────────────────
    if (millis() - lastSonarCheck >= US_IDLE_INTERVAL) {
        lastSonarCheck = millis();
        bool nearby = sonar.isPersonNearby(WAKE_DISTANCE_CM);

        if (!nearby) {
            // 距離超過門檻，開始計時
            unsigned long idleSec = (millis() - lastActivityTime) / 1000;
            if (idleSec >= SLEEP_TIMEOUT_SEC) {
                Serial.printf("💤 %lu 秒無人，進入休眠\n",
                              (unsigned long)SLEEP_TIMEOUT_SEC);

                // 清理畫面後關閉 OLED
                ui.showMessage("Standby...", "Entering sleep");
                delay(800);
                ui.display.ssd1306_command(SSD1306_DISPLAYOFF);

                setLED(false, false);
                inputBuf     = "";
                currentState = STATE_SLEEP;
                return;
            }
        } else {
            // 有人在附近，更新活動時間
            markActivity();
        }
    }

    // ── OLED 待機畫面（每秒更新）────────────────────
    if (millis() - lastOledUpdate > 1000) {
        lastOledUpdate = millis();

        // 確保 OLED 是開啟狀態
        ui.display.ssd1306_command(SSD1306_DISPLAYON);

        auto b = battery.getStatus();
        ui.showIdle(getCurrentTime(), weatherCache.temp,
                    weatherCache.rainToday, b.percentage, b.charging);
    }

    // ── 鍵盤 ────────────────────────────────────────
    char key = scanKeypad(keypadPCF);
    if (key) handleKeyInput(key);

    // ── 指紋輪詢 ─────────────────────────────────────
    if (FINGERPRINT_EN && fpOK) {
        int fpId = verifyFingerprint();
        if (fpId > 0) {
            successUnlock("Fingerprint #" + String(fpId));
        } else if (fpId == -1) {
            failedAttempt();
        }
    }

    // ── 人臉辨識（限速）─────────────────────────────
    if (FACE_RECOGNITION_EN) {
        camera_fb_t* fb = faceSystem.capture();
        if (fb) {
            if (pendingEnroll) {
                checkPendingEnroll(fb);
                ui.showEnrollProgress(pendingEnrollName,
                    pendingEnrollCount, FACE_ENROLL_SAMPLES);
            } else {
                ui.showVerifying("Face Recognition");
                auto res = faceSystem.process(fb);

                if (res.recognized) {
                    markActivity();
                    successUnlock(res.name, fb);
                } else if (res.face_detected && STRANGER_ALERT_EN) {
                    static unsigned long lastAlert = 0;
                    if (millis() - lastAlert > 30000) {
                        lastAlert = millis();
                        markActivity();
                        Serial.println("⚠️ 陌生人偵測");
                        sendTelegramPhoto(fb, "⚠️ 陌生人！\n" + getCurrentDateTime());
                    }
                }
            }
            faceSystem.returnFrame(fb);
        }
    }
}

// ════════════════════════════════════════════
//  STATE_UNLOCKED：已解鎖倒數
// ════════════════════════════════════════════

void handleUnlocked() {
    updateRelay();

    static int lastSec = -1;
    int sec = unlockRemainingSec();
    if (sec != lastSec) {
        lastSec = sec;
        ui.showUnlocked("Welcome!", getWeatherShort(weatherCache), sec);
    }

    if (!isDoorOpen()) {
        currentState = STATE_IDLE;   // 鎖門後回到 IDLE（有人在附近）
        setLED(false, false);
        markActivity();
        lastSec = -1;
    }
}

// ════════════════════════════════════════════
//  STATE_ALARM：警報
// ════════════════════════════════════════════

void handleAlarm() {
    ui.showAlarm();
    static unsigned long lastBeep = 0;
    if (millis() - lastBeep > 2000) {
        lastBeep = millis();
        playSoundAsync(SOUND_ALARM);
        setLED(false, (millis() / 350) % 2);
    }

    char key = scanKeypad(keypadPCF);
    if (!key) return;
    markActivity();

    if (key == '*') {
        inputBuf = "";
    } else if (key == '#') {
        if (inputBuf == currentPassword || inputBuf == ADMIN_PASSWORD) {
            failCount    = 0;
            currentState = STATE_IDLE;
            setLED(false, false);
            sendTelegramMessage("ℹ️ 警報已由本地解除");
        }
        inputBuf = "";
    } else if (isDigit(key)) {
        inputBuf += key;
    }
}

// ════════════════════════════════════════════
//  STATE_FACE_MGMT：人臉管理選單
// ════════════════════════════════════════════

void handleFaceMgmt() {
    ui.showFaceMenu(faceSystem.getCount());
    markActivity();

    char key = waitForKey(keypadPCF, 15000);
    markActivity();

    if (key == '1') {
        ui.showMessage("Add Face", "Use Telegram:\n/face_enroll [name]");
        sendTelegramMessage("📷 請使用 Telegram 登錄：\n/face_enroll 姓名");
        delay(3000);
    } else if (key == '2') {
        auto list = faceSystem.getList();
        ui.showList("Faces (del via TG)", list);
        sendTelegramMessage("/face_list 查看，/face_delete 姓名 刪除");
        delay(4000);
    } else if (key == '3') {
        auto list = faceSystem.getList();
        if (list.empty()) ui.showMessage("No faces", "Database empty");
        else              ui.showList("Enrolled Faces", list);
        delay(4000);
    } else if (key == '4') {
        ui.showMessage("Confirm?", "Press # to delete ALL");
        char c = waitForKey(keypadPCF, 5000);
        if (c == '#') {
            faceSystem.deleteAll();
            faceDB.removeAll();
            ui.showMessage("Done", "All faces deleted");
            playSoundAsync(SOUND_DENY);
            delay(2000);
        }
    }

    if (key == '*' || key == 0) currentState = STATE_IDLE;
}

// ════════════════════════════════════════════
//  setup
// ════════════════════════════════════════════

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("╔════════════════════════════╗");
    Serial.println("║   智慧門鎖 V2  啟動中...  ║");
    Serial.println("╚════════════════════════════╝");

    // 1. I2C + OLED
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    ui.begin();
    ui.showMessage("Booting...", "Init hardware");

    // 2. 基礎硬體
    initRelay();
    keypadPCF.begin();
    statusPCF.begin();
    setLED(false, false);
    battery.begin();

    // 3. HC-SR04 超聲波感測器
    sonar.begin();
    float testDist = sonar.measureMedianCm(3);
    Serial.printf("HC-SR04 測試距離：%.1f cm\n", testDist);

    // 4. 鏡頭
    ui.showMessage("Booting...", "Camera init");
    if (!faceSystem.initCamera()) {
        ui.showMessage("ERROR", "Camera failed!");
        delay(3000);
    }

    // 5. SPIFFS
    ui.showMessage("Booting...", "Load face DB");
    faceDB.begin();

    // 6. 指紋
    ui.showMessage("Booting...", "Fingerprint");
    initFingerprint();

    // 7. 音頻
    initAudio();

    // 8. WiFi + NTP
    ui.showMessage("Booting...", "Connect WiFi");
    bool wifiOK = connectWiFi();
    if (wifiOK) {
        syncTime();
        weatherCache      = getWeather();
        lastWeatherUpdate = millis();
    }

    // 9. 啟動完成 → 進入 SLEEP 等待有人靠近
    playSoundAsync(SOUND_STARTUP);
    setLED(false, false);

    // OLED 顯示休眠訊息後關閉螢幕
    ui.showMessage("Ready.", "Waiting nearby...");
    delay(1500);
    ui.display.ssd1306_command(SSD1306_DISPLAYOFF);

    currentState     = STATE_SLEEP;
    lastActivityTime = millis();

    if (wifiOK) {
        sendTelegramMessage(
            "🔐 智慧門鎖已啟動（近接喚醒模式）\n"
            "IP: " + WiFi.localIP().toString() + "\n"
            "喚醒距離: " + String(WAKE_DISTANCE_CM) + " cm\n"
            "電量: " + battery.toShortString() + "\n" +
            getCurrentDateTime()
        );
    }

    Serial.println("✅ 啟動完成，進入休眠等待有人靠近...");
}

// ════════════════════════════════════════════
//  loop
// ════════════════════════════════════════════

void loop() {
    // ── 週期性任務（不受狀態影響）────────────────────

    // WiFi 維持
    maintainWiFi();

    // Telegram 輪詢（3 秒，SLEEP 狀態下仍執行）
    if (millis() - lastBotPoll > BOT_POLL_MS) {
        lastBotPoll = millis();
        handleTelegramCommands();
        // 若 Telegram 下達 /unlock，強制喚醒
        if (currentState == STATE_SLEEP && isDoorOpen()) {
            ui.display.ssd1306_command(SSD1306_DISPLAYON);
            currentState   = STATE_UNLOCKED;
            markActivity();
        }
    }

    // 天氣更新（10 分鐘）
    if (millis() - lastWeatherUpdate > WEATHER_UPDATE_MS) {
        lastWeatherUpdate = millis();
        if (WiFi.status() == WL_CONNECTED)
            weatherCache = getWeather();
    }

    // 電池監控（30 秒）
    if (millis() - lastBattCheck > 30000) {
        lastBattCheck = millis();
        auto b = battery.getStatus(true);
        if (b.lowBattery) {
            static bool alerted = false;
            if (!alerted) {
                alerted = true;
                if (currentState != STATE_SLEEP) playSoundAsync(SOUND_LOW_BATT);
                sendTelegramMessage("⚡ 電量不足：" + String(b.percentage) + "%");
            }
        }
        if (b.percentage < CRITICAL_BATTERY_PCT) setCpuFrequencyMhz(80);
    }

    // ── 狀態機 ────────────────────────────────────────
    switch (currentState) {
        case STATE_SLEEP:      handleSleep();     break;
        case STATE_IDLE:       handleIdle();      break;
        case STATE_UNLOCKED:   handleUnlocked();  break;
        case STATE_ALARM:      handleAlarm();     break;
        case STATE_FACE_MGMT:  handleFaceMgmt();  break;
    }
}
```

---

## U7. 單獨測試程式

在整合前，先單獨驗證 HC-SR04 是否正常工作：

```cpp
// 測試程式：HC-SR04 超聲波距離量測
// 上傳此程式，在序列埠監視器觀察距離輸出

#include <Arduino.h>
#include <Wire.h>
#include <PCF8574.h>

#define I2C_SDA  5
#define I2C_SCL  6
#define ECHO_PIN 9         // GPIO9 = D10
#define TRIG_P   3         // PCF8574 #2 P3
PCF8574 statusPCF(0x21, I2C_SDA, I2C_SCL);

float measureCm() {
    // TRIG via PCF8574
    statusPCF.write(TRIG_P, HIGH);
    statusPCF.write(TRIG_P, LOW);

    // 等待 ECHO 上升沿
    unsigned long t = micros();
    while (digitalRead(ECHO_PIN) == LOW) {
        if (micros() - t > 5000) return -1;
    }
    // 量測 ECHO 持續時間
    unsigned long start = micros();
    while (digitalRead(ECHO_PIN) == HIGH) {
        if (micros() - start > 25000) return -1;
    }
    return (micros() - start) * 0.034f / 2.0f;
}

void setup() {
    Serial.begin(115200);
    Wire.begin(I2C_SDA, I2C_SCL);
    statusPCF.begin();
    statusPCF.write(TRIG_P, LOW);
    pinMode(ECHO_PIN, INPUT);
    delay(2000);
    Serial.println("=== HC-SR04 距離量測測試 ===");
    Serial.println("逐漸靠近感測器，觀察距離變化");
    Serial.println("距離 <= 80cm 時系統應喚醒");
    Serial.println("");
}

void loop() {
    // 取三次中位數
    float readings[3];
    for (int i = 0; i < 3; i++) {
        readings[i] = measureCm();
        delay(30);
    }
    // 排序取中位數
    if (readings[0] > readings[1]) { float t=readings[0]; readings[0]=readings[1]; readings[1]=t; }
    if (readings[1] > readings[2]) { float t=readings[1]; readings[1]=readings[2]; readings[2]=t; }
    if (readings[0] > readings[1]) { float t=readings[0]; readings[0]=readings[1]; readings[1]=t; }
    float dist = readings[1];

    if (dist < 0) {
        Serial.println("量測失敗（距離太遠 或 接線問題）");
    } else {
        Serial.printf("距離：%6.1f cm  %s\n",
            dist,
            dist <= 80 ? "【★ 系統應喚醒】" : "[ 休眠中 ]");
    }

    delay(300);
}
```

### 預期輸出

```
=== HC-SR04 距離量測測試 ===
逐漸靠近感測器，觀察距離變化
距離 <=80cm 時系統應喚醒

距離：  213.4 cm  [ 休眠中 ]
距離：  198.7 cm  [ 休眠中 ]
距離：  142.2 cm  [ 休眠中 ]
距離：   98.5 cm  [ 休眠中 ]
距離：   74.3 cm  【★ 系統應喚醒】
距離：   52.1 cm  【★ 系統應喚醒】
距離：   38.8 cm  【★ 系統應喚醒】
```

---

## U8. 測試清單（新增）

在完成原有 A~E 階段測試後，追加以下測試：

```
▶ 階段 F：超聲波近接喚醒

□ F1  HC-SR04 硬體確認
      □ I2C 掃描仍顯示 0x20、0x21、0x3C（PCF8574 #2 未受影響）
      □ 單獨測試程式量測距離合理（拿尺比對，誤差應 < 2cm）
      □ 確認 ECHO 分壓電路：用電表量 D10 腳位電壓 < 3.4V

□ F2  距離量測準確性
      □ 將物體放在 50cm 處，讀值為 45~55cm（±10% 誤差正常）
      □ 距離超過 4m 時回傳 -1（無回波）
      □ 連續量測不會讓系統死鎖

□ F3  休眠喚醒功能
      □ 系統啟動後進入 STATE_SLEEP，OLED 關閉，序列埠印 "等待靠近"
      □ 走到感測器前（距離 ≤ 80cm），OLED 亮起、發出嗶聲
      □ 序列埠印 "偵測到有人靠近（XX cm），喚醒系統"
      □ 系統進入 STATE_IDLE，人臉辨識和指紋開始運作

□ F4  自動休眠功能
      □ 系統在 IDLE 狀態下，離開感測器範圍（> 80cm）
      □ 等待 15 秒後 OLED 自動關閉，序列埠印 "進入休眠"
      □ 重新靠近可再次喚醒

□ F5  休眠期間 Telegram 仍可用
      □ 系統在 SLEEP 狀態下，傳送 /status 可正確回應
      □ 傳送 /unlock，門鎖動作，OLED 自動亮起顯示解鎖畫面

□ F6  解鎖後不進入休眠
      □ 解鎖成功後，在 UNLOCKED 倒數 5 秒期間不會進入休眠
      □ 警報狀態不會進入休眠

□ F7  調校測試
      □ 修改 WAKE_DISTANCE_CM = 50，確認只在 50cm 內才喚醒
      □ 修改 SLEEP_TIMEOUT_SEC = 5，確認 5 秒無人就休眠
      □ 測試完畢，恢復 80cm 和 15s
```

---

## U9. 調校建議

### 偵測距離設定

根據門口環境調整 `WAKE_DISTANCE_CM`：

| 環境 | 建議距離 | 說明 |
|------|---------|------|
| 狹窄走廊（< 1m 寬）| 60~70 cm | 避免走廊另一側觸發 |
| 一般門口 | 70~90 cm | 預設值，適合大多數場景 |
| 寬敞玄關 | 80~120 cm | 讓使用者走近時有足夠時間準備 |
| 室外（風雨環境）| 50~60 cm | HC-SR04 在戶外受風干擾，縮小範圍 |

### 感測器安裝位置

```
✅ 建議安裝位置：
   門框側邊（高度 1.0~1.5m）
   朝向走廊/門外，角度與地面平行

❌ 避免：
   正對玻璃門（超聲波穿透或反射異常）
   朝向熱空氣出口（冷氣、暖氣導管）
   有水流/雨水可能打到感測器的位置
   距離牆角 < 30cm（旁瓣反射干擾）
```

### 假觸發排除

若系統在無人時也頻繁喚醒：

```cpp
// 在 config.h 中調整：

// 方法 1：縮小偵測距離
#define WAKE_DISTANCE_CM    60   // 從 80 縮小到 60

// 方法 2：增加確認次數（需要多次連續偵測到才喚醒）
// 在 ultrasonic.h 的 isPersonNearby() 中改為：
// return (measureMedianCm(5) <= thresholdCm);  // 5 次中位數

// 方法 3：縮短休眠超時（快速重新休眠）
#define SLEEP_TIMEOUT_SEC   8    // 從 15 秒縮短為 8 秒
```

---

## U10. 常見問題

### Q1：量測值跳動很大（例如 80cm 和 200cm 交替出現）

```
原因：ECHO 信號不穩定
解決：
  1. 確認分壓電阻焊接正確（量 D10 的電壓應在 3.0~3.4V 之間）
  2. ECHO 線路不要太長（< 20cm），避免拾取雜訊
  3. 使用 measureMedianCm(5) 取 5 次中位數（更穩定）
  4. HC-SR04 量測間隔至少 30ms，不能太快
```

### Q2：TRIG 通過 PCF8574 後量測距離總是偏大

```
原因：I2C 操作有延遲，TRIG 脈衝後 ECHO 上升沿已過
解決：
  通常不會發生，因為 HC-SR04 在收到 TRIG 後約 500μs 才發出 ECHO
  而我們的 pulseIn 在第二次 I2C write 完成後立刻開始等待（< 200μs）
  如果仍有問題，在第二個 write 後加一個 delayMicroseconds(200)
```

### Q3：系統喚醒後人臉辨識很慢，感覺反應遲鈍

```
原因：鏡頭從 PSRAM 初始化需要幾十毫秒
解決：
  鏡頭並不需要在休眠時真正關閉，只需要停止 fb_get 即可
  在 STATE_SLEEP 時只是不呼叫 faceSystem.capture()，鏡頭本身保持初始化狀態
  喚醒後第一幀可能較慢，第二幀起速度正常
  若想加快首幀：在喚醒後先 capture() 一次 dummy frame 並丟棄
```

### Q4：休眠後 OLED 完全關閉，重新喚醒時短暫白屏

```
原因：SSD1306_DISPLAYON 指令發出到顯示器首次更新之間有約 100ms 空白
解決：這是正常現象，SSD1306 規格書標準行為
     喚醒時先呼叫 display.ssd1306_command(SSD1306_DISPLAYON)
     再立刻呼叫 display.display() 推送內容，可減少白屏時間
     在 oled_ui.h 的 showMessage() 開頭加入此行即可
```

---

*HC-SR04 近接喚醒章節完 ｜ 需整合進主文件第九章之前，並更新第三十四章的 platformio.ini 與第三十八章的腳位速查表*

---


# 第四部分：逐模組測試（Phase 1）


> 💡 **嵌入式開發守則**：永遠先單獨測試每個模組，確認正常後再整合。一次整合所有東西只會讓除錯變得非常困難。

---

## 第九章：I2C 掃描測試

**目標**：確認 OLED（0x3C）、PCF8574 #1（0x20）、PCF8574 #2（0x21）都能被正確偵測到。

### 接線確認
- SDA：D4（GPIO5）連接所有 I2C 裝置的 SDA，並透過 4.7kΩ 上拉到 3.3V
- SCL：D5（GPIO6）連接所有 I2C 裝置的 SCL，並透過 4.7kΩ 上拉到 3.3V
- 所有模組的 VCC 接 3.3V、GND 接 GND

### 測試程式

```cpp
// src/main.cpp — I2C 地址掃描
#include <Arduino.h>
#include <Wire.h>

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("=== I2C 地址掃描 ===");

    // 初始化 I2C：SDA=GPIO5, SCL=GPIO6
    Wire.begin(5, 6);

    int found = 0;
    for (uint8_t addr = 0x01; addr < 0x7F; addr++) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            Serial.printf("✅ 找到裝置：0x%02X", addr);
            // 辨識已知地址
            if (addr == 0x20) Serial.print("  ← PCF8574 #1（鍵盤）");
            if (addr == 0x21) Serial.print("  ← PCF8574 #2（LED）");
            if (addr == 0x3C) Serial.print("  ← SSD1306 OLED");
            Serial.println();
            found++;
        }
    }

    Serial.printf("\n共找到 %d 個裝置\n", found);
    if (found < 3) {
        Serial.println("❌ 預期找到 3 個裝置，請檢查接線！");
    } else {
        Serial.println("✅ 所有 I2C 裝置正常！");
    }
}

void loop() {}
```

### 預期輸出

```
=== I2C 地址掃描 ===
✅ 找到裝置：0x20  ← PCF8574 #1（鍵盤）
✅ 找到裝置：0x21  ← PCF8574 #2（LED）
✅ 找到裝置：0x3C  ← SSD1306 OLED

共找到 3 個裝置
✅ 所有 I2C 裝置正常！
```

### 排除問題

| 症狀 | 原因 | 解決 |
|------|------|------|
| 完全找不到裝置 | SDA/SCL 接反，或缺少上拉電阻 | 確認 D4=SDA, D5=SCL，加 4.7kΩ 上拉 |
| 只找到部分裝置 | 特定模組未供電或地址跳線設定錯誤 | 個別檢查每個模組的 VCC/GND 和地址腳位 |
| 地址衝突 | 兩個 PCF8574 地址相同 | 確認 PCF8574 #2 的 A0 接 VCC（非 GND）|

---

## 第十章：OLED 顯示器測試

**目標**：確認 OLED 可以正常顯示文字和圖形。

### 測試程式

```cpp
// src/main.cpp — OLED 基本測試
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_WIDTH   128
#define OLED_HEIGHT  64
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

void setup() {
    Serial.begin(115200);
    delay(2000);

    Wire.begin(5, 6);  // SDA=GPIO5, SCL=GPIO6

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println("❌ OLED 初始化失敗！");
        while (1) delay(1000);
    }

    Serial.println("✅ OLED 初始化成功");

    // ── 畫面 1：文字測試 ──
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("SmartLock");

    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println("XIAO ESP32 S3 Sense");
    display.setCursor(0, 32);
    display.println("Face / FP / Password");
    display.setCursor(0, 44);
    display.println("System OK!");
    display.display();

    delay(3000);

    // ── 畫面 2：圖形測試 ──
    display.clearDisplay();
    display.drawRect(0, 0, 128, 64, SSD1306_WHITE);     // 外框
    display.drawLine(0, 32, 128, 32, SSD1306_WHITE);    // 水平線
    display.drawCircle(64, 32, 20, SSD1306_WHITE);       // 圓形
    display.fillCircle(64, 32, 5, SSD1306_WHITE);        // 實心圓
    display.display();
}

void loop() {
    // 動畫：計時顯示
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 10);
    display.printf("T=%lu s", millis() / 1000);
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.print("OLED Working!");
    display.display();
    delay(1000);
}
```

### 預期結果

- 啟動後 OLED 顯示 "SmartLock" 等文字 3 秒
- 接著顯示幾何圖形測試
- 之後進入 loop，每秒更新計時器數字

### 排除問題

| 症狀 | 解決 |
|------|------|
| 白屏（全亮）| OLED 地址可能是 0x3D，修改 OLED_ADDRESS |
| 完全無反應 | 先通過第九章 I2C 掃描確認 0x3C 存在 |
| 花屏 | I2C 線路太長，縮短或加強上拉電阻（2.2kΩ）|
| 顯示翻轉 | 部分模組需要 `display.setRotation(2)` |

---

## 第十一章：4×4 鍵盤測試

**目標**：確認所有 16 個按鍵都能被正確識別。

### 測試程式

```cpp
// src/main.cpp — 4×4 鍵盤掃描測試
#include <Arduino.h>
#include <Wire.h>
#include <PCF8574.h>

// PCF8574 #1 地址 0x20，SDA=GPIO5, SCL=GPIO6
PCF8574 keypad(0x20, 5, 6);

// 鍵盤字元對應表
const char keyMap[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// 掃描鍵盤，回傳按下的字元，無按鍵回傳 0
char scanKeypad() {
    for (int row = 0; row < 4; row++) {
        // 把當前 row 設為 LOW，其他 row 設為 HIGH
        for (int r = 0; r < 4; r++) {
            keypad.write(r, (r == row) ? LOW : HIGH);
        }
        delayMicroseconds(200);  // 等訊號穩定

        // 讀取所有 col（P4~P7）
        for (int col = 0; col < 4; col++) {
            if (keypad.read(col + 4) == LOW) {
                // 防彈跳：等待 50ms 後再確認
                delay(50);
                if (keypad.read(col + 4) != LOW) continue;

                // 等待按鍵放開
                while (keypad.read(col + 4) == LOW) delay(10);
                return keyMap[row][col];
            }
        }
    }
    return 0;
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    Wire.begin(5, 6);
    keypad.begin();
    Serial.println("=== 鍵盤測試 ===");
    Serial.println("請按任意按鍵...");
    Serial.println("依序按完所有 16 個按鍵確認功能正常");
}

void loop() {
    char key = scanKeypad();
    if (key) {
        Serial.printf("按下：[ %c ]\n", key);
    }
    delay(20);
}
```

### 預期輸出（依序按 1~9, 0, *, #, A~D）

```
=== 鍵盤測試 ===
請按任意按鍵...
按下：[ 1 ]
按下：[ 2 ]
按下：[ 3 ]
按下：[ A ]
...以此類推，共 16 個按鍵
```

### 排除問題

| 症狀 | 原因 | 解決 |
|------|------|------|
| 按鍵顯示錯誤字元 | 鍵盤排線接錯順序 | 調換 Row 或 Col 的接線，或修改 keyMap |
| 某些按鍵無反應 | 排線接觸不良 | 重新插拔排線到 PCF8574 腳位 |
| 按一次觸發多次 | 防彈跳不足 | 增加 delay(50) 到 delay(80) |

---

## 第十二章：AS608 指紋模組測試

**目標**：成功登錄一個指紋，並驗證辨識正確、陌生手指不通過。

### 接線再確認

```
AS608 VCC → 3.3V（不能接 5V！）
AS608 GND → GND
AS608 TXD → ESP32 D7 (GPIO44)  ← ESP32 的 RX
AS608 RXD → ESP32 D6 (GPIO43)  ← ESP32 的 TX
```

### 測試程式

```cpp
// src/main.cpp — AS608 指紋測試（登錄 + 驗證）
#include <Arduino.h>
#include <Adafruit_Fingerprint.h>

// 使用 UART1，RX=GPIO44, TX=GPIO43
HardwareSerial mySerial(1);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
    Serial.begin(115200);
    delay(2000);

    // 初始化 AS608
    mySerial.begin(57600, SERIAL_8N1, 44, 43);
    delay(100);

    finger.begin(57600);

    Serial.println("=== AS608 指紋模組測試 ===");

    if (finger.verifyPassword()) {
        Serial.println("✅ 找到指紋感測器！");
        finger.getParameters();
        Serial.printf("型號: 0x%04X\n", finger.system_id);
        Serial.printf("容量: %d 筆\n", finger.capacity);
        Serial.printf("安全等級: %d\n", finger.security_level);
    } else {
        Serial.println("❌ 未找到指紋感測器，請檢查接線！");
        while (1) delay(1000);
    }

    Serial.println("\n--- 指令 ---");
    Serial.println("輸入 'E' + Enter：登錄指紋到 ID=1");
    Serial.println("輸入 'V' + Enter：驗證指紋");
    Serial.println("輸入 'C' + Enter：清空所有指紋");
}

// 登錄指紋（ID 固定為 1 做測試）
bool enrollFP() {
    int id = 1;
    Serial.printf("\n=== 登錄指紋 ID #%d ===\n", id);

    Serial.println("【第一次】請放上手指...");
    while (finger.getImage() != FINGERPRINT_OK) delay(50);

    if (finger.image2Tz(1) != FINGERPRINT_OK) {
        Serial.println("❌ 特徵提取失敗，請重試");
        return false;
    }
    Serial.println("✅ 第一次掃描成功！請移開手指");
    delay(2000);
    while (finger.getImage() != FINGERPRINT_NOFINGER) delay(50);

    Serial.println("【第二次】請再次放上同一根手指...");
    while (finger.getImage() != FINGERPRINT_OK) delay(50);

    if (finger.image2Tz(2) != FINGERPRINT_OK) {
        Serial.println("❌ 特徵提取失敗");
        return false;
    }

    if (finger.createModel() != FINGERPRINT_OK) {
        Serial.println("❌ 兩次指紋不匹配，請重試");
        return false;
    }

    if (finger.storeModel(id) != FINGERPRINT_OK) {
        Serial.println("❌ 儲存失敗");
        return false;
    }

    Serial.printf("✅ 指紋 #%d 登錄成功！\n", id);
    return true;
}

// 驗證指紋
void verifyFP() {
    Serial.println("\n請放上手指驗證...");

    // 等待手指
    unsigned long t = millis();
    while (finger.getImage() != FINGERPRINT_OK) {
        if (millis() - t > 10000) {
            Serial.println("逾時，取消");
            return;
        }
        delay(50);
    }

    if (finger.image2Tz() != FINGERPRINT_OK) {
        Serial.println("❌ 特徵提取失敗");
        return;
    }

    if (finger.fingerFastSearch() == FINGERPRINT_OK) {
        if (finger.confidence >= 40) {
            Serial.printf("✅ 辨識成功！ID=%d, 信心=%d\n",
                          finger.fingerID, finger.confidence);
        } else {
            Serial.printf("⚠️ 匹配但信心太低（%d < 40），視為不匹配\n",
                          finger.confidence);
        }
    } else {
        Serial.println("❌ 不匹配（陌生手指）");
    }
}

void loop() {
    if (Serial.available()) {
        char c = Serial.read();
        if (c == 'E' || c == 'e') enrollFP();
        else if (c == 'V' || c == 'v') verifyFP();
        else if (c == 'C' || c == 'c') {
            finger.emptyDatabase();
            Serial.println("✅ 所有指紋已清空");
        }
    }
    delay(100);
}
```

### 測試流程

1. 上傳後開啟序列埠監視器（鮑率 115200）
2. 輸入 `E` 按 Enter，按照提示登錄你的食指（第一次→移開→第二次）
3. 看到「登錄成功！」後，輸入 `V` 按 Enter，放上食指驗證
4. 再次輸入 `V`，放上不同手指，確認顯示「不匹配」

---

## 第十三章：MAX98357 音頻測試

**目標**：確認喇叭能清楚播放不同頻率的聲音。

### 測試程式

```cpp
// src/main.cpp — I2S 音頻輸出測試
#include <Arduino.h>
#include <driver/i2s.h>
#include <math.h>

#define I2S_PORT      I2S_NUM_0
#define I2S_BCLK_PIN  2    // D1
#define I2S_LRCLK_PIN 3    // D2
#define I2S_DATA_PIN  4    // D3
#define SAMPLE_RATE   22050

void setupI2S() {
    i2s_config_t cfg = {
        .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate          = SAMPLE_RATE,
        .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count        = 4,
        .dma_buf_len          = 256,
        .use_apll             = false,
        .tx_desc_auto_clear   = true
    };
    i2s_pin_config_t pins = {
        .bck_io_num   = I2S_BCLK_PIN,
        .ws_io_num    = I2S_LRCLK_PIN,
        .data_out_num = I2S_DATA_PIN,
        .data_in_num  = I2S_PIN_NO_CHANGE
    };
    i2s_driver_install(I2S_PORT, &cfg, 0, NULL);
    i2s_set_pin(I2S_PORT, &pins);
}

// 播放單一音符（頻率 Hz，持續 ms）
void playTone(int freqHz, int durMs, int volume = 8000) {
    if (freqHz <= 0) { delay(durMs); return; }
    int totalSamples = SAMPLE_RATE * durMs / 1000;
    int16_t buf[512];
    int written = 0;
    while (written < totalSamples) {
        int chunk = min(256, totalSamples - written);
        int16_t stereo[512];
        for (int i = 0; i < chunk; i++) {
            int16_t s = (int16_t)(volume * sinf(2.0f * M_PI * freqHz * (written + i) / SAMPLE_RATE));
            stereo[i * 2]     = s;
            stereo[i * 2 + 1] = s;
        }
        size_t bw;
        i2s_write(I2S_PORT, stereo, chunk * 4, &bw, portMAX_DELAY);
        written += chunk;
    }
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    setupI2S();
    Serial.println("=== MAX98357 音頻測試 ===");
}

void loop() {
    Serial.println("播放解鎖音效...");
    playTone(784, 80);   // G5
    playTone(988, 80);   // B5
    playTone(1319, 150); // E6

    delay(1000);

    Serial.println("播放錯誤音效...");
    playTone(350, 200);
    playTone(250, 300);

    delay(1000);

    Serial.println("播放警報音效...");
    for (int i = 0; i < 3; i++) {
        playTone(2000, 150);
        playTone(0, 100);  // 靜音
    }

    delay(2000);
}
```

### 排除問題

| 症狀 | 解決 |
|------|------|
| 完全無聲 | 確認 VIN 接 5V（非 3.3V）、BCLK/LRC/DIN 腳位正確 |
| 嗡嗡聲（雜訊）| 電源不穩，加 100μF 電容在 VIN 和 GND 間 |
| 聲音很小 | 增加 `volume` 參數到 12000；或把 GAIN 腳位接 GND（12dB 增益）|

---

## 第十四章：繼電器與電磁鎖測試

**目標**：確認繼電器可被控制，電磁鎖正確動作。

> ⚠️ 此步驟需要 12V 電源，請確認 12V 接線完全正確再通電。

### 測試程式

```cpp
// src/main.cpp — 繼電器控制測試
#include <Arduino.h>

#define RELAY_PIN 1  // GPIO1 = D0

void setup() {
    Serial.begin(115200);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);  // 初始：門鎖緊
    delay(2000);
    Serial.println("=== 繼電器測試 ===");
    Serial.println("輸入 'O' 開鎖 3 秒，輸入 'L' 立刻鎖門");
    Serial.println("初始狀態：門已鎖緊");
}

void loop() {
    if (Serial.available()) {
        char c = toupper(Serial.read());
        if (c == 'O') {
            Serial.println("🔓 開鎖中...");
            digitalWrite(RELAY_PIN, LOW);   // 繼電器動作 = 電磁鎖斷電 = 門開
            delay(3000);
            digitalWrite(RELAY_PIN, HIGH);  // 繼電器復位 = 電磁鎖通電 = 門鎖
            Serial.println("🔒 已自動鎖門");
        } else if (c == 'L') {
            digitalWrite(RELAY_PIN, HIGH);
            Serial.println("🔒 已鎖門");
        }
    }
}
```

### 測試流程

1. **不接 12V**：輸入 O，確認聽到繼電器「喀」一聲（繼電器本身動作聲）
2. **接上 12V 和電磁鎖**：輸入 O，確認電磁鎖釋放（門可以開），3 秒後自動鎖緊

---

## 第十五章：鏡頭與人臉辨識測試

### 15.1 鏡頭基本測試

**目標**：確認 PSRAM 已啟用，鏡頭初始化成功。

```cpp
// src/main.cpp — 鏡頭基本測試
#include <Arduino.h>
#include "esp_camera.h"

// XIAO ESP32 S3 Sense 鏡頭腳位（固定，勿修改）
#define PWDN_GPIO_NUM  -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  10
#define SIOD_GPIO_NUM  40
#define SIOC_GPIO_NUM  39
#define Y9_GPIO_NUM    48
#define Y8_GPIO_NUM    11
#define Y7_GPIO_NUM    12
#define Y6_GPIO_NUM    14
#define Y5_GPIO_NUM    16
#define Y4_GPIO_NUM    18
#define Y3_GPIO_NUM    17
#define Y2_GPIO_NUM    15
#define VSYNC_GPIO_NUM 38
#define HREF_GPIO_NUM  47
#define PCLK_GPIO_NUM  13

bool initCamera() {
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;
    config.pin_d0  = Y2_GPIO_NUM;
    config.pin_d1  = Y3_GPIO_NUM;
    config.pin_d2  = Y4_GPIO_NUM;
    config.pin_d3  = Y5_GPIO_NUM;
    config.pin_d4  = Y6_GPIO_NUM;
    config.pin_d5  = Y7_GPIO_NUM;
    config.pin_d6  = Y8_GPIO_NUM;
    config.pin_d7  = Y9_GPIO_NUM;
    config.pin_xclk     = XCLK_GPIO_NUM;
    config.pin_pclk     = PCLK_GPIO_NUM;
    config.pin_vsync    = VSYNC_GPIO_NUM;
    config.pin_href     = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn     = PWDN_GPIO_NUM;
    config.pin_reset    = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    // 人臉辨識必須使用 RGB565 格式
    config.pixel_format = PIXFORMAT_RGB565;
    config.frame_size   = FRAMESIZE_240X240;
    config.fb_location  = CAMERA_FB_IN_PSRAM;
    config.fb_count     = 2;
    config.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("❌ 鏡頭初始化失敗，錯誤碼: 0x%x\n", err);
        return false;
    }

    // 調整影像品質
    sensor_t* s = esp_camera_sensor_get();
    s->set_hmirror(s, 1);       // 水平鏡像
    s->set_brightness(s, 1);    // 亮度 +1
    s->set_contrast(s, 1);      // 對比 +1
    s->set_whitebal(s, 1);      // 自動白平衡
    s->set_exposure_ctrl(s, 1); // 自動曝光

    return true;
}

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.printf("PSRAM: %s\n", psramFound() ? "✅ 已找到" : "❌ 未找到");
    Serial.printf("PSRAM 大小: %d MB\n", ESP.getPsramSize() / 1024 / 1024);

    if (!psramFound()) {
        Serial.println("❌ PSRAM 未找到！請確認 platformio.ini 設定正確");
        while (1) delay(1000);
    }

    if (!initCamera()) {
        while (1) delay(1000);
    }
    Serial.println("✅ 鏡頭初始化成功！");
}

void loop() {
    camera_fb_t* fb = esp_camera_fb_get();
    if (fb) {
        Serial.printf("✅ 拍攝成功 | 解析度: %dx%d | 大小: %d bytes | 格式: RGB565\n",
                      fb->width, fb->height, fb->len);
        esp_camera_fb_return(fb);
    } else {
        Serial.println("❌ 拍攝失敗");
    }
    delay(2000);
}
```

**預期輸出**：
```
PSRAM: ✅ 已找到
PSRAM 大小: 8 MB
✅ 鏡頭初始化成功！
✅ 拍攝成功 | 解析度: 240x240 | 大小: 115200 bytes | 格式: RGB565
```

### 15.2 人臉偵測測試

```cpp
// src/main.cpp — 人臉偵測測試（需要 esp-face）
#include <Arduino.h>
#include "esp_camera.h"
#include "human_face_detect_msr01.hpp"
#include "human_face_detect_mnp01.hpp"

// 鏡頭腳位定義（同上，略）
// ...（貼上 15.1 的 initCamera 函式）

HumanFaceDetectMSR01 s1(0.3F, 0.3F, 10, 0.3F);
HumanFaceDetectMNP01 s2(0.4F, 0.3F, 10);

void setup() {
    Serial.begin(115200);
    delay(2000);
    if (!initCamera()) while (1) delay(1000);
    Serial.println("✅ 準備就緒，請站到鏡頭前...");
}

void loop() {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) { delay(100); return; }

    // Stage 1：快速偵測
    auto candidates = s1.infer(
        (uint16_t*)fb->buf,
        {(int)fb->height, (int)fb->width, 3}
    );

    if (!candidates.empty()) {
        // Stage 2：精確定位
        auto faces = s2.infer(
            (uint16_t*)fb->buf,
            {(int)fb->height, (int)fb->width, 3},
            candidates
        );

        if (!faces.empty()) {
            Serial.printf("✅ 偵測到 %d 張人臉\n", faces.size());
            for (auto& f : faces) {
                Serial.printf("   邊框：x=%d y=%d w=%d h=%d\n",
                    (int)f.box[0], (int)f.box[1],
                    (int)(f.box[2] - f.box[0]),
                    (int)(f.box[3] - f.box[1]));
            }
        } else {
            Serial.println("候選框存在但精化失敗");
        }
    } else {
        Serial.println("無人臉");
    }

    esp_camera_fb_return(fb);
    delay(300);
}
```

---

## 第十六章：電池電源測試

**目標**：確認電池電壓讀取正確，充電狀態偵測正常。

### 測試程式

```cpp
// src/main.cpp — 電池監控測試
#include <Arduino.h>

#define BATT_ADC_PIN  8   // GPIO8 = D9
#define BATT_CHRG_PIN 7   // GPIO7 = D8

void setup() {
    Serial.begin(115200);
    delay(2000);

    pinMode(BATT_ADC_PIN, INPUT);
    pinMode(BATT_CHRG_PIN, INPUT_PULLUP);
    analogSetAttenuation(ADC_11db);  // 支援到 3.3V 輸入

    Serial.println("=== 電池監控測試 ===");
    Serial.println("每 2 秒顯示一次電池狀態");
}

float readBatteryVoltage() {
    // 多次取樣平均（降低 ADC 雜訊）
    long sum = 0;
    for (int i = 0; i < 16; i++) {
        sum += analogRead(BATT_ADC_PIN);
        delay(2);
    }
    int raw = sum / 16;

    // 換算：ADC 值 → 電壓
    // 分壓比 0.5，ADC 基準 3.3V，解析度 12-bit（0~4095）
    float adcV = raw / 4095.0f * 3.3f;
    float battV = adcV * 2.0f;  // 乘以分壓倍數

    Serial.printf("ADC raw: %d, ADC電壓: %.3fV, 電池電壓: %.3fV\n",
                  raw, adcV, battV);
    return battV;
}

int voltageToPercent(float v) {
    if (v >= 4.2f) return 100;
    if (v >= 4.0f) return map(v * 100, 400, 420, 75, 100);
    if (v >= 3.7f) return map(v * 100, 370, 400, 40, 75);
    if (v >= 3.5f) return map(v * 100, 350, 370, 15, 40);
    if (v >= 3.3f) return map(v * 100, 330, 350,  5, 15);
    return 0;
}

void loop() {
    float v = readBatteryVoltage();
    int pct = voltageToPercent(v);
    bool charging = (digitalRead(BATT_CHRG_PIN) == LOW);

    Serial.printf("電池：%.2fV | %d%% | %s\n",
                  v, pct, charging ? "充電中 ⚡" : "放電中");

    if (pct < 20) {
        Serial.println("⚠️ 電量偏低，請充電！");
    }

    delay(2000);
}
```

### 校準提示

用三用電表直接量電池電壓（B+ 和 GND 之間），比較程式讀到的值：
- 若程式讀值偏高：分壓電阻實際比值偏小（換算時乘數改為 2.1 或 2.2）
- 若程式讀值偏低：換算時乘數改為 1.9 或 1.8

---

# 第五部分：雲端功能（Phase 2）

---

## 第十七章：WiFi 連線與 NTP 時間

**目標**：連上 WiFi，並從網路同步台灣時間，讓系統知道現在幾點。

### 測試程式

```cpp
// src/main.cpp — WiFi 連線 + NTP 時間同步測試
#include <Arduino.h>
#include <WiFi.h>

const char* WIFI_SSID = "你的WiFi名稱";    // ← 修改這裡
const char* WIFI_PASS = "你的WiFi密碼";    // ← 修改這裡

void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("=== WiFi + NTP 測試 ===");

    // 連接 WiFi
    Serial.printf("連接 WiFi：%s ...", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    int attempt = 0;
    while (WiFi.status() != WL_CONNECTED && attempt < 40) {
        delay(500);
        Serial.print(".");
        attempt++;
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\n❌ WiFi 連線失敗！");
        Serial.println("請確認：1) SSID/密碼正確  2) 路由器是 2.4GHz  3) 訊號強度足夠");
        return;
    }

    Serial.printf("\n✅ WiFi 連線成功！\n");
    Serial.printf("IP 位址: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("訊號強度: %d dBm\n", WiFi.RSSI());

    // 同步 NTP 時間（UTC+8 台灣）
    configTime(8 * 3600, 0, "pool.ntp.org", "time.asia.apple.com");
    Serial.print("同步 NTP 時間...");

    struct tm timeinfo;
    int retry = 0;
    while (!getLocalTime(&timeinfo) && retry < 20) {
        delay(500);
        Serial.print(".");
        retry++;
    }

    if (getLocalTime(&timeinfo)) {
        char buf[32];
        strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M:%S", &timeinfo);
        Serial.printf("\n✅ 時間同步成功：%s\n", buf);
    } else {
        Serial.println("\n⚠️ NTP 同步失敗，請檢查網路連線");
    }
}

void loop() {
    struct tm t;
    if (getLocalTime(&t)) {
        char buf[32];
        strftime(buf, sizeof(buf), "%H:%M:%S", &t);
        Serial.printf("現在時間：%s | WiFi RSSI: %d dBm\n",
                      buf, WiFi.RSSI());
    }
    delay(5000);
}
```

### 預期輸出

```
=== WiFi + NTP 測試 ===
連接 WiFi：MyHomeWiFi ...........
✅ WiFi 連線成功！
IP 位址: 192.168.1.105
訊號強度: -52 dBm
同步 NTP 時間.....
✅ 時間同步成功：2025/05/03 14:32:18
現在時間：14:32:23 | WiFi RSSI: -52 dBm
```

### 排除問題

| 症狀 | 解決方式 |
|------|---------|
| 一直印 "." 無法連線 | 確認密碼正確（區分大小寫）；路由器確認是 2.4GHz |
| IP 為 0.0.0.0 | 路由器 DHCP 未分配，重啟路由器 |
| NTP 失敗 | 先確認 WiFi IP 正常，再檢查防火牆是否阻擋 UDP 123 port |

---

## 第十八章：天氣 API 整合

### 18.1 申請 OpenWeatherMap API Key

1. 前往 https://openweathermap.org/api 點 **Sign Up** 免費註冊
2. 登入後點右上角帳號 → **My API keys**
3. 預設已有一組 Key，點「Copy」複製
4. 新 Key 需要等待約 1~2 小時才能生效

### 18.2 測試程式

```cpp
// src/main.cpp — 天氣 API 測試
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* WIFI_SSID   = "你的WiFi名稱";
const char* WIFI_PASS   = "你的WiFi密碼";
const char* OWM_API_KEY = "你的OpenWeatherMap_APIKey";  // ← 修改
const char* OWM_CITY    = "Taipei,TW";

void setup() {
    Serial.begin(115200);
    delay(2000);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println("\n✅ WiFi 已連線");

    // 建立請求 URL
    String url = "http://api.openweathermap.org/data/2.5/weather?q=";
    url += String(OWM_CITY);
    url += "&appid=" + String(OWM_API_KEY);
    url += "&units=metric&lang=zh_tw";

    HTTPClient http;
    http.setTimeout(8000);
    http.begin(url);
    int code = http.GET();

    Serial.printf("HTTP 狀態碼: %d\n", code);

    if (code == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("原始 JSON（前 200 字）：");
        Serial.println(payload.substring(0, 200));

        // 解析 JSON
        StaticJsonDocument<1024> doc;
        if (!deserializeJson(doc, payload)) {
            String desc   = doc["weather"][0]["description"].as<String>();
            float  temp   = doc["main"]["temp"].as<float>();
            int    humid  = doc["main"]["humidity"].as<int>();
            float  wind   = doc["wind"]["speed"].as<float>();
            String main   = doc["weather"][0]["main"].as<String>();

            Serial.println("\n=== 台北天氣 ===");
            Serial.printf("天氣描述：%s\n", desc.c_str());
            Serial.printf("溫度：%.1f°C\n", temp);
            Serial.printf("濕度：%d%%\n", humid);
            Serial.printf("風速：%.1f m/s\n", wind);
            Serial.printf("是否下雨：%s\n",
                (main == "Rain" || main == "Drizzle" || main == "Thunderstorm")
                ? "是 ☔" : "否");
        }
    } else if (code == 401) {
        Serial.println("❌ 錯誤 401：API Key 無效或尚未生效（新 Key 需等 1 小時）");
    } else {
        Serial.printf("❌ 請求失敗，HTTP 錯誤碼：%d\n", code);
    }
    http.end();
}

void loop() {}
```

---

## 第十九章：Telegram Bot 設定

### 19.1 建立 Telegram Bot

**Step 1：找到 BotFather**
1. 開啟 Telegram，搜尋 `@BotFather`（官方機器人，藍色勾勾認證）
2. 點擊開啟並按 **START**

**Step 2：建立 Bot**
1. 傳送 `/newbot`
2. 輸入 Bot 的顯示名稱（例如：`MySmartLock`）
3. 輸入 Bot 的用戶名（必須以 `bot` 結尾，例如：`MySmartLock_bot`）
4. BotFather 會回傳一組 **Token**（格式：`7891234567:AAHxxx...`），複製保存

**Step 3：取得你的 Chat ID**
1. 在 Telegram 搜尋你剛建立的 Bot 名稱，點開並按 **START**
2. 傳送任意訊息（例如 `hello`）
3. 在瀏覽器開啟以下網址（把 YOUR_TOKEN 換成你的 Token）：
   ```
   https://api.telegram.org/botYOUR_TOKEN/getUpdates
   ```
4. 在回傳的 JSON 中找到 `"chat":{"id":XXXXXXX}`，那串數字就是你的 Chat ID

### 19.2 測試程式

```cpp
// src/main.cpp — Telegram Bot 測試
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

const char* WIFI_SSID = "你的WiFi名稱";
const char* WIFI_PASS = "你的WiFi密碼";
const char* BOT_TOKEN = "你的BotToken";    // ← 修改
const String CHAT_ID  = "你的ChatID";      // ← 修改（純數字）

WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

void setup() {
    Serial.begin(115200);
    delay(2000);

    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println("\n✅ WiFi 已連線");

    client.setInsecure();  // 開發用，跳過 SSL 驗證

    // 傳送測試訊息
    Serial.println("傳送 Telegram 測試訊息...");
    bool ok = bot.sendMessage(CHAT_ID,
        "🔐 智慧門鎖 Telegram 測試成功！\n"
        "IP: " + WiFi.localIP().toString(), "");

    if (ok) {
        Serial.println("✅ 訊息傳送成功！請查看 Telegram");
    } else {
        Serial.println("❌ 訊息傳送失敗，請確認 Token 和 Chat ID");
    }
}

void loop() {
    // 接收並回應指令
    client.setInsecure();
    int msgCount = bot.getUpdates(bot.last_message_received + 1);

    for (int i = 0; i < msgCount; i++) {
        String text   = bot.messages[i].text;
        String fromId = bot.messages[i].chat_id;

        Serial.printf("收到來自 %s 的訊息：%s\n", fromId.c_str(), text.c_str());

        if (fromId != CHAT_ID) {
            bot.sendMessage(fromId, "⛔ 未授權", "");
            continue;
        }

        if (text == "/ping") {
            bot.sendMessage(CHAT_ID, "🏓 Pong！系統正常運作中", "");
        } else if (text == "/help") {
            bot.sendMessage(CHAT_ID,
                "/ping - 測試連線\n/help - 說明", "");
        } else {
            bot.sendMessage(CHAT_ID, "收到：" + text, "");
        }
    }

    delay(3000);  // 每 3 秒輪詢一次
}
```

---

# 第六部分：完整原始碼（Phase 3）

---

## 第二十章：專案目錄結構

完成所有測試後，在 `src/` 目錄下建立以下檔案：

```
SmartLock/
├── platformio.ini              ← 已完成（第八章）
├── partitions_smartlock.csv    ← 已完成（第八章）
├── data/
│   └── faces/                  ← SPIFFS 自動建立，存放人臉特徵
└── src/
    ├── main.cpp                ← 第三十三章
    ├── config.h                ← 第二十一章  ⚠️ 唯一需要填入個人資訊的檔案
    ├── face_recognition.h      ← 第二十二章
    ├── face_database.h         ← 第二十三章
    ├── oled_ui.h               ← 第二十四章
    ├── battery.h               ← 第二十五章
    ├── keypad.h                ← 第二十六章
    ├── fingerprint.h           ← 第二十七章
    ├── audio.h                 ← 第二十八章
    ├── relay.h                 ← 第二十九章
    ├── wifi_mgr.h              ← 第三十章
    ├── weather.h               ← 第三十一章
    └── telegram_bot.h          ← 第三十二章
```

> 💡 **建立方式**：在 VS Code 的 Explorer 中，右鍵點擊 `src` 資料夾 → `New File`，輸入檔案名稱即可。

---

## 第二十一章：config.h

> ⚠️ **這是唯一需要修改個人資訊的檔案**，其他所有 `.h` 檔案只需複製貼上，不需要修改。

```cpp
// src/config.h
#pragma once

// ════════════════════════════════════════
//  WiFi 設定
// ════════════════════════════════════════
#define WIFI_SSID           "你的WiFi名稱"      // ← 修改
#define WIFI_PASS           "你的WiFi密碼"      // ← 修改
#define WIFI_TIMEOUT_SEC    20

// ════════════════════════════════════════
//  Telegram Bot
// ════════════════════════════════════════
#define BOT_TOKEN           "你的BotToken"      // ← 修改（從 @BotFather 取得）
#define CHAT_ID             "你的ChatID"        // ← 修改（純數字）
#define BOT_POLL_MS         3000

// ════════════════════════════════════════
//  天氣 API
// ════════════════════════════════════════
#define OWM_API_KEY         "你的OWM_APIKey"    // ← 修改（從 openweathermap.org 取得）
#define OWM_CITY            "Taipei"
#define OWM_COUNTRY         "TW"
#define OWM_LANG            "zh_tw"
#define WEATHER_UPDATE_MS   (10 * 60 * 1000UL)

// ════════════════════════════════════════
//  NTP 時間
// ════════════════════════════════════════
#define NTP_SERVER1         "pool.ntp.org"
#define NTP_SERVER2         "time.asia.apple.com"
#define TZ_OFFSET_SEC       (8 * 3600)
#define DST_OFFSET_SEC      0

// ════════════════════════════════════════
//  密碼設定（上線前務必修改！）
// ════════════════════════════════════════
#define DEFAULT_PASSWORD    "1234"     // ← 修改為你的密碼（4~8位數字）
#define ADMIN_PASSWORD      "9999"     // ← 修改為管理員密碼（與一般密碼不同）
#define MAX_PASSWORD_LEN    8
#define MAX_FAIL_ATTEMPTS   5

// ════════════════════════════════════════
//  硬體腳位（根據接線圖，通常不需修改）
// ════════════════════════════════════════
#define I2C_SDA_PIN         5    // D4
#define I2C_SCL_PIN         6    // D5
#define AS608_TX_PIN        43   // D6 → AS608 RX
#define AS608_RX_PIN        44   // D7 ← AS608 TX
#define AS608_BAUD          57600
#define I2S_BCLK_PIN        2    // D1
#define I2S_LRCLK_PIN       3    // D2
#define I2S_DATA_PIN        4    // D3
#define RELAY_PIN           1    // D0
#define BATT_ADC_PIN        8    // D9
#define BATT_CHRG_PIN       7    // D8

// ════════════════════════════════════════
//  PCF8574 I2C 地址與腳位
// ════════════════════════════════════════
#define PCF_KEYPAD_ADDR     0x20
#define PCF_STATUS_ADDR     0x21
#define LED_GREEN_P         0    // PCF8574 #2 的 P0
#define LED_RED_P           1    // PCF8574 #2 的 P1
#define BUZZER_P            2    // PCF8574 #2 的 P2（選配）

// ════════════════════════════════════════
//  OLED
// ════════════════════════════════════════
#define OLED_ADDRESS        0x3C
#define OLED_WIDTH          128
#define OLED_HEIGHT         64

// ════════════════════════════════════════
//  功能開關
// ════════════════════════════════════════
#define FACE_RECOGNITION_EN  true
#define STRANGER_ALERT_EN    true
#define WEATHER_NOTIFY_EN    true
#define FINGERPRINT_EN       true

// ════════════════════════════════════════
//  行為設定
// ════════════════════════════════════════
#define UNLOCK_DURATION_MS      5000
#define FACE_SCAN_INTERVAL_MS   300
#define LOW_BATTERY_PCT         20
#define CRITICAL_BATTERY_PCT    10
#define MAX_FACE_COUNT          10
#define FACE_ENROLL_SAMPLES     5

// ════════════════════════════════════════
//  鏡頭腳位（固定，勿修改）
// ════════════════════════════════════════
#define PWDN_GPIO_NUM   -1
#define RESET_GPIO_NUM  -1
#define XCLK_GPIO_NUM   10
#define SIOD_GPIO_NUM   40
#define SIOC_GPIO_NUM   39
#define Y9_GPIO_NUM     48
#define Y8_GPIO_NUM     11
#define Y7_GPIO_NUM     12
#define Y6_GPIO_NUM     14
#define Y5_GPIO_NUM     16
#define Y4_GPIO_NUM     18
#define Y3_GPIO_NUM     17
#define Y2_GPIO_NUM     15
#define VSYNC_GPIO_NUM  38
#define HREF_GPIO_NUM   47
#define PCLK_GPIO_NUM   13

// ════════════════════════════════════════
//  系統狀態機（跨檔案共用）
// ════════════════════════════════════════
enum SystemState {
    STATE_IDLE,
    STATE_UNLOCKED,
    STATE_ALARM,
    STATE_FACE_MGMT
};
```

---

## 第二十二章：face_recognition.h

```cpp
// src/face_recognition.h
#pragma once

#include <Arduino.h>
#include <vector>
#include "esp_camera.h"
#include "human_face_detect_msr01.hpp"
#include "human_face_detect_mnp01.hpp"
#include "face_recognition_112_v1_s16.hpp"
#include "config.h"

class FaceRecognitionSystem {
public:
    HumanFaceDetectMSR01    detector1;
    HumanFaceDetectMNP01    detector2;
    FaceRecognition112V1S16 recognizer;

    struct Result {
        bool   face_detected;
        bool   recognized;
        String name;
    };

    FaceRecognitionSystem()
        : detector1(0.3F, 0.3F, 10, 0.3F),
          detector2(0.4F, 0.3F, 10)
    {}

    // ── 初始化鏡頭（RGB565 模式）──────────────────
    bool initCamera() {
        camera_config_t cfg;
        cfg.ledc_channel  = LEDC_CHANNEL_0;
        cfg.ledc_timer    = LEDC_TIMER_0;
        cfg.pin_d0  = Y2_GPIO_NUM;  cfg.pin_d1 = Y3_GPIO_NUM;
        cfg.pin_d2  = Y4_GPIO_NUM;  cfg.pin_d3 = Y5_GPIO_NUM;
        cfg.pin_d4  = Y6_GPIO_NUM;  cfg.pin_d5 = Y7_GPIO_NUM;
        cfg.pin_d6  = Y8_GPIO_NUM;  cfg.pin_d7 = Y9_GPIO_NUM;
        cfg.pin_xclk     = XCLK_GPIO_NUM;
        cfg.pin_pclk     = PCLK_GPIO_NUM;
        cfg.pin_vsync    = VSYNC_GPIO_NUM;
        cfg.pin_href     = HREF_GPIO_NUM;
        cfg.pin_sscb_sda = SIOD_GPIO_NUM;
        cfg.pin_sscb_scl = SIOC_GPIO_NUM;
        cfg.pin_pwdn     = PWDN_GPIO_NUM;
        cfg.pin_reset    = RESET_GPIO_NUM;
        cfg.xclk_freq_hz = 20000000;
        cfg.pixel_format = PIXFORMAT_RGB565;
        cfg.frame_size   = FRAMESIZE_240X240;
        cfg.fb_location  = CAMERA_FB_IN_PSRAM;
        cfg.fb_count     = 2;
        cfg.grab_mode    = CAMERA_GRAB_WHEN_EMPTY;

        if (!psramFound()) {
            Serial.println("❌ PSRAM 未找到！");
            return false;
        }

        if (esp_camera_init(&cfg) != ESP_OK) {
            Serial.println("❌ 鏡頭初始化失敗");
            return false;
        }

        sensor_t* s = esp_camera_sensor_get();
        s->set_hmirror(s, 1);
        s->set_brightness(s, 1);
        s->set_contrast(s, 1);
        s->set_whitebal(s, 1);
        s->set_exposure_ctrl(s, 1);

        Serial.println("✅ 鏡頭初始化完成（RGB565 240×240）");
        return true;
    }

    camera_fb_t* capture() {
        return esp_camera_fb_get();
    }

    void returnFrame(camera_fb_t* fb) {
        if (fb) esp_camera_fb_return(fb);
    }

    // ── 偵測並辨識人臉 ────────────────────────────
    Result process(camera_fb_t* fb) {
        Result r = {false, false, ""};
        if (!fb || fb->format != PIXFORMAT_RGB565) return r;

        auto candidates = detector1.infer(
            (uint16_t*)fb->buf,
            {(int)fb->height, (int)fb->width, 3}
        );
        if (candidates.empty()) return r;

        r.face_detected = true;

        auto faces = detector2.infer(
            (uint16_t*)fb->buf,
            {(int)fb->height, (int)fb->width, 3},
            candidates
        );
        if (faces.empty()) return r;

        int id = recognizer.recognize(
            (uint16_t*)fb->buf,
            {(int)fb->height, (int)fb->width, 3},
            faces.front().keypoint
        );

        if (id >= 0) {
            r.recognized = true;
            auto list = recognizer.get_enrolled_id_list();
            if (id < (int)list.size()) {
                r.name = String(list[id].c_str());
            }
        }
        return r;
    }

    // ── 登錄人臉（單張）────────────────────────────
    bool enrollOne(camera_fb_t* fb, const String& name) {
        if (!fb || fb->format != PIXFORMAT_RGB565) return false;

        auto candidates = detector1.infer(
            (uint16_t*)fb->buf,
            {(int)fb->height, (int)fb->width, 3}
        );
        if (candidates.empty()) return false;

        auto faces = detector2.infer(
            (uint16_t*)fb->buf,
            {(int)fb->height, (int)fb->width, 3},
            candidates
        );
        if (faces.empty()) return false;

        int id = recognizer.enroll_id(
            (uint16_t*)fb->buf,
            {(int)fb->height, (int)fb->width, 3},
            faces.front().keypoint,
            name.c_str(),
            true
        );
        return (id >= 0);
    }

    // ── 刪除指定名稱的人臉 ────────────────────────
    bool deleteByName(const String& name) {
        auto list = recognizer.get_enrolled_id_list();
        for (int i = 0; i < (int)list.size(); i++) {
            if (String(list[i].c_str()) == name) {
                recognizer.delete_id(i);
                return true;
            }
        }
        return false;
    }

    // ── 清除所有人臉 ──────────────────────────────
    void deleteAll() {
        int n = recognizer.get_enrolled_count();
        for (int i = n - 1; i >= 0; i--) recognizer.delete_id(i);
    }

    // ── 取得已登錄清單 ────────────────────────────
    std::vector<String> getList() {
        std::vector<String> result;
        for (auto& s : recognizer.get_enrolled_id_list())
            result.push_back(String(s.c_str()));
        return result;
    }

    int getCount() {
        return recognizer.get_enrolled_count();
    }
};
```

---

## 第二十三章：face_database.h

```cpp
// src/face_database.h
#pragma once

#include <SPIFFS.h>
#include <vector>
#include "config.h"

#define FACE_DB_DIR  "/faces"
#define FACE_MAGIC   0x46414345UL

// 記錄使用者名稱（特徵向量由辨識器管理，此處負責名稱持久化）
class FaceDatabase {
public:
    bool begin() {
        if (!SPIFFS.begin(true)) {
            Serial.println("❌ SPIFFS 掛載失敗");
            return false;
        }
        if (!SPIFFS.exists(FACE_DB_DIR)) {
            SPIFFS.mkdir(FACE_DB_DIR);
        }
        Serial.printf("✅ SPIFFS 掛載完成 | 可用: %d KB\n",
            (SPIFFS.totalBytes() - SPIFFS.usedBytes()) / 1024);
        return true;
    }

    // 儲存名稱標記（登錄成功後呼叫）
    bool saveName(const String& name) {
        String path = String(FACE_DB_DIR) + "/" + name + ".name";
        File f = SPIFFS.open(path, FILE_WRITE);
        if (!f) return false;
        f.write((uint8_t*)name.c_str(), name.length());
        f.close();
        return true;
    }

    // 讀取所有已存名稱
    std::vector<String> loadNames() {
        std::vector<String> names;
        File dir = SPIFFS.open(FACE_DB_DIR);
        if (!dir || !dir.isDirectory()) return names;
        File entry = dir.openNextFile();
        while (entry) {
            if (!entry.isDirectory()) {
                String fname = String(entry.name());
                // 去掉副檔名 .name
                int dot = fname.lastIndexOf('.');
                if (dot > 0) fname = fname.substring(0, dot);
                names.push_back(fname);
            }
            entry = dir.openNextFile();
        }
        return names;
    }

    // 刪除指定名稱
    bool removeName(const String& name) {
        String path = String(FACE_DB_DIR) + "/" + name + ".name";
        return SPIFFS.remove(path);
    }

    // 清除所有
    void removeAll() {
        File dir = SPIFFS.open(FACE_DB_DIR);
        if (!dir || !dir.isDirectory()) return;
        std::vector<String> paths;
        File e = dir.openNextFile();
        while (e) {
            paths.push_back(String(FACE_DB_DIR) + "/" + String(e.name()));
            e = dir.openNextFile();
        }
        for (auto& p : paths) SPIFFS.remove(p);
    }

    int count() { return loadNames().size(); }
};
```

---

## 第二十四章：oled_ui.h

```cpp
// src/oled_ui.h
#pragma once

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <vector>
#include "config.h"

class OledUI {
public:
    Adafruit_SSD1306 display;
    bool   alarmVisible   = true;
    uint8_t spinFrame     = 0;
    unsigned long lastSpin  = 0;
    unsigned long lastBlink = 0;

    OledUI() : display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1) {}

    bool begin() {
        if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
            Serial.println("❌ OLED 初始化失敗");
            return false;
        }
        display.clearDisplay();
        display.setTextColor(SSD1306_WHITE);
        display.cp437(true);
        Serial.println("✅ OLED 初始化完成");
        return true;
    }

    // ── 待機畫面 ──────────────────────────────────
    void showIdle(const String& timeStr, float temp,
                  bool rain, int battPct, bool charging) {
        display.clearDisplay();

        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print("  SMART LOCK  V2");
        display.drawLine(0, 9, 127, 9, SSD1306_WHITE);

        display.setTextSize(2);
        display.setCursor(20, 13);
        display.print(timeStr);

        display.setTextSize(1);
        display.setCursor(0, 33);
        if (rain)
            display.printf("%.0fC  ☔ 帶傘！", temp);
        else
            display.printf("%.0fC  Clear", temp);

        display.drawLine(0, 43, 127, 43, SSD1306_WHITE);
        drawBatteryBar(0, 46, battPct, charging);

        display.setCursor(0, 57);
        display.print("Face / FP / Password");
        display.display();
    }

    // ── 驗證中（轉動動畫）────────────────────────
    void showVerifying(const String& method) {
        const char* frames[] = {"|","/" ,"-","\\","|","/","-","\\"};
        if (millis() - lastSpin > 120) {
            spinFrame = (spinFrame + 1) % 8;
            lastSpin = millis();
        }
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(15, 0);
        display.print("Verifying...");
        display.setTextSize(3);
        display.setCursor(52, 18);
        display.print(frames[spinFrame]);
        display.setTextSize(1);
        display.setCursor(0, 55);
        display.print(method.substring(0, 21));
        display.display();
    }

    // ── 解鎖成功 ──────────────────────────────────
    void showUnlocked(const String& name, const String& weather,
                      int remainSec) {
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.print("OPEN");
        display.setTextSize(1);
        display.setCursor(55, 5);
        display.print("Welcome!");
        display.drawLine(0, 18, 127, 18, SSD1306_WHITE);
        display.setCursor(0, 22);
        display.print(name.substring(0, 21));
        display.setCursor(0, 34);
        display.print(weather.substring(0, 21));
        display.drawLine(0, 46, 127, 46, SSD1306_WHITE);
        display.setCursor(0, 50);
        display.printf("Auto-lock in %ds...", remainSec);
        display.display();
    }

    // ── 驗證失敗 ──────────────────────────────────
    void showDenied(int failCnt, int maxFail) {
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.print("DENIED");
        display.setTextSize(1);
        display.setCursor(0, 22);
        display.printf("Fail: %d / %d", failCnt, maxFail);
        display.setCursor(0, 34);
        display.printf("Left: %d attempts", maxFail - failCnt);
        display.setCursor(0, 55);
        display.print("[*]Clear  [#]Confirm");
        display.display();
    }

    // ── 警報（閃爍）──────────────────────────────
    void showAlarm() {
        if (millis() - lastBlink > 350) {
            alarmVisible = !alarmVisible;
            lastBlink = millis();
        }
        display.clearDisplay();
        if (alarmVisible) {
            display.setTextSize(2);
            display.setCursor(5, 5);
            display.print("!! ALARM !!");
        }
        display.setTextSize(1);
        display.setCursor(0, 30);
        display.print("Too many failures!");
        display.setCursor(0, 42);
        display.print("Enter admin password");
        display.setCursor(0, 55);
        display.print("[*]Clear  [#]Confirm");
        display.display();
    }

    // ── 密碼輸入 ──────────────────────────────────
    void showPasswordInput(int len, const String& prompt = "Enter Password") {
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print(prompt.substring(0, 21));
        display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
        display.setTextSize(2);
        display.setCursor(10, 18);
        String dots = "";
        for (int i = 0; i < len && i < 8; i++) dots += "*";
        display.print(dots.length() > 0 ? dots : "_");
        display.setTextSize(1);
        display.setCursor(0, 44);
        display.printf("Digits entered: %d", len);
        display.setCursor(0, 55);
        display.print("[*]Clear  [#]Confirm");
        display.display();
    }

    // ── 人臉管理選單 ──────────────────────────────
    void showFaceMenu(int faceCnt) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.printf("Face Mgmt  [%d/10]", faceCnt);
        display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
        display.setCursor(0, 14); display.print("1. Add face (TG cmd)");
        display.setCursor(0, 24); display.print("2. Delete face");
        display.setCursor(0, 34); display.print("3. List all faces");
        display.setCursor(0, 44); display.print("4. Delete ALL");
        display.setCursor(0, 55); display.print("[*]Back to main");
        display.display();
    }

    // ── 登錄人臉進度 ──────────────────────────────
    void showEnrollProgress(const String& name, int done, int total) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print("Enrolling face...");
        display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
        display.setCursor(0, 14);
        display.print(name.substring(0, 16));
        display.setCursor(0, 26);
        display.printf("Sample %d of %d", done, total);
        // 進度條
        display.drawRect(0, 38, 128, 10, SSD1306_WHITE);
        int fill = done * 126 / total;
        if (fill > 0) display.fillRect(1, 39, fill, 8, SSD1306_WHITE);
        display.setCursor(0, 55);
        display.print("Look at the camera!");
        display.display();
    }

    // ── 清單顯示 ──────────────────────────────────
    void showList(const String& title, const std::vector<String>& items,
                  int page = 0) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print(title.substring(0, 21));
        display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
        int start = page * 4;
        for (int i = 0; i < 4 && start + i < (int)items.size(); i++) {
            display.setCursor(0, 14 + i * 12);
            display.printf("%d.%s", start + i + 1,
                           items[start + i].substring(0, 18).c_str());
        }
        if ((int)items.size() > 4) {
            display.setCursor(0, 55);
            display.printf("Pg %d/%d [#]next",
                           page+1, (items.size()+3)/4);
        }
        display.display();
    }

    // ── 通用訊息 ──────────────────────────────────
    void showMessage(const String& title, const String& msg) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.print(title.substring(0, 21));
        display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
        display.setCursor(0, 20);
        display.print(msg.substring(0, 21));
        display.setCursor(0, 34);
        display.print(msg.substring(21, 42));
        display.display();
    }

    // ── 低電量警告 ────────────────────────────────
    void showLowBattery(int pct) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(18, 0);
        display.print("!! LOW BATTERY !!");
        display.drawLine(0, 10, 127, 10, SSD1306_WHITE);
        display.setTextSize(3);
        display.setCursor(28, 18);
        display.printf("%d%%", pct);
        display.setTextSize(1);
        display.setCursor(10, 50);
        display.print("Please charge now!");
        display.display();
    }

private:
    void drawBatteryBar(int x, int y, int pct, bool charging) {
        display.drawRect(x, y, 60, 8, SSD1306_WHITE);
        display.drawRect(x+60, y+2, 3, 4, SSD1306_WHITE);
        int fill = pct * 58 / 100;
        if (fill > 0) display.fillRect(x+1, y+1, fill, 6, SSD1306_WHITE);
        display.setTextSize(1);
        display.setCursor(x+66, y);
        if (charging) display.print("CHG");
        else display.printf("%d%%", pct);
    }
};
```

---

## 第二十五章：battery.h

```cpp
// src/battery.h
#pragma once
#include <Arduino.h>
#include "config.h"

class BatteryMonitor {
public:
    struct Status {
        int   raw;
        float voltage;
        int   percentage;
        bool  charging;
        bool  lowBattery;
    };

    Status cached = {};
    unsigned long lastCheck = 0;

    void begin() {
        pinMode(BATT_ADC_PIN, INPUT);
        pinMode(BATT_CHRG_PIN, INPUT_PULLUP);
        analogSetAttenuation(ADC_11db);
        Serial.println("✅ 電池監控初始化完成");
    }

    Status getStatus(bool force = false) {
        if (!force && millis() - lastCheck < 30000) return cached;
        lastCheck = millis();

        long sum = 0;
        for (int i = 0; i < 16; i++) { sum += analogRead(BATT_ADC_PIN); delay(2); }
        int raw = sum / 16;

        float adcV  = raw / 4095.0f * 3.3f;
        float battV = adcV * 2.0f;
        int   pct   = voltToPercent(battV);
        bool  chrg  = (digitalRead(BATT_CHRG_PIN) == LOW);

        cached = { raw, battV, pct, chrg, (pct < LOW_BATTERY_PCT) };
        return cached;
    }

    String toShortString() {
        auto s = getStatus();
        char buf[20];
        if (s.charging) snprintf(buf, sizeof(buf), "CHG %.1fV", s.voltage);
        else            snprintf(buf, sizeof(buf), "%d%% %.1fV", s.percentage, s.voltage);
        return String(buf);
    }

private:
    int voltToPercent(float v) {
        if (v >= 4.20f) return 100;
        if (v >= 4.00f) return (int)(75  + (v - 4.00f) / 0.20f * 25);
        if (v >= 3.70f) return (int)(40  + (v - 3.70f) / 0.30f * 35);
        if (v >= 3.50f) return (int)(15  + (v - 3.50f) / 0.20f * 25);
        if (v >= 3.30f) return (int)(5   + (v - 3.30f) / 0.20f * 10);
        return 0;
    }
};
```

---

## 第二十六章：keypad.h

```cpp
// src/keypad.h
#pragma once
#include <PCF8574.h>

static const char KEY_MAP[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

char scanKeypad(PCF8574& pcf) {
    for (int row = 0; row < 4; row++) {
        for (int r = 0; r < 4; r++)
            pcf.write(r, (r == row) ? LOW : HIGH);
        delayMicroseconds(150);
        for (int col = 0; col < 4; col++) {
            if (pcf.read(col + 4) == LOW) {
                delay(50);  // 防彈跳
                if (pcf.read(col + 4) != LOW) continue;
                unsigned long t = millis();
                while (pcf.read(col + 4) == LOW && millis()-t < 3000) delay(10);
                return KEY_MAP[row][col];
            }
        }
    }
    return 0;
}

char waitForKey(PCF8574& pcf, unsigned long timeoutMs = 10000) {
    unsigned long start = millis();
    while (millis() - start < timeoutMs) {
        char k = scanKeypad(pcf);
        if (k) return k;
        delay(50);
    }
    return 0;
}
```

---

## 第二十七章：fingerprint.h

```cpp
// src/fingerprint.h
#pragma once
#include <Adafruit_Fingerprint.h>
#include "config.h"

extern HardwareSerial    fpSerial;
extern Adafruit_Fingerprint finger;
bool fpOK = false;

bool initFingerprint() {
    fpSerial.begin(AS608_BAUD, SERIAL_8N1, AS608_RX_PIN, AS608_TX_PIN);
    delay(200);
    finger.begin(AS608_BAUD);
    if (finger.verifyPassword()) {
        fpOK = true;
        finger.getParameters();
        Serial.printf("✅ AS608 就緒 | 容量:%d | 安全等級:%d\n",
                      finger.capacity, finger.security_level);
        return true;
    }
    Serial.println("⚠️ AS608 未偵測到，指紋功能停用");
    return false;
}

// 驗證：回傳 ID（>0=成功，-1=不匹配，-2=無法掃描）
int verifyFingerprint() {
    if (!fpOK) return -2;
    if (finger.getImage()        != FINGERPRINT_OK) return -2;
    if (finger.image2Tz()        != FINGERPRINT_OK) return -2;
    if (finger.fingerFastSearch()!= FINGERPRINT_OK) return -1;
    if (finger.confidence < 40)                      return -1;
    return finger.fingerID;
}

// 登錄：id=1~127
bool enrollFingerprint(int id) {
    if (!fpOK) return false;
    Serial.printf("=== 登錄指紋 #%d ===\n", id);

    // 第一次掃描
    Serial.println("請放上手指...");
    unsigned long t = millis();
    while (finger.getImage() != FINGERPRINT_OK) {
        if (millis()-t > 10000) return false;
        delay(50);
    }
    if (finger.image2Tz(1) != FINGERPRINT_OK) return false;
    Serial.println("✅ 第一次完成，移開手指...");
    delay(1500);
    while (finger.getImage() != FINGERPRINT_NOFINGER) delay(50);

    // 第二次掃描
    Serial.println("請再次放上手指...");
    t = millis();
    while (finger.getImage() != FINGERPRINT_OK) {
        if (millis()-t > 10000) return false;
        delay(50);
    }
    if (finger.image2Tz(2)   != FINGERPRINT_OK) return false;
    if (finger.createModel()  != FINGERPRINT_OK) return false;
    if (finger.storeModel(id) != FINGERPRINT_OK) return false;

    Serial.printf("✅ 指紋 #%d 登錄成功\n", id);
    return true;
}

bool deleteFingerprint(int id) {
    if (!fpOK) return false;
    return finger.deleteModel(id) == FINGERPRINT_OK;
}

bool clearAllFingerprints() {
    if (!fpOK) return false;
    return finger.emptyDatabase() == FINGERPRINT_OK;
}
```

---

## 第二十八章：audio.h

```cpp
// src/audio.h
#pragma once
#include <driver/i2s.h>
#include <math.h>
#include "config.h"

#define I2S_PORT       I2S_NUM_0
#define SAMPLE_RATE    22050
#define AUDIO_VOL      8000

struct ToneNote { int freq; int dur; };
static const ToneNote SOUND_STARTUP[]   = {{880,80},{1175,80},{1760,120},{0,0}};
static const ToneNote SOUND_UNLOCK[]    = {{784,80},{988,80},{1319,150},{0,0}};
static const ToneNote SOUND_DENY[]      = {{350,200},{250,300},{0,0}};
static const ToneNote SOUND_BEEP[]      = {{1200,60},{0,0}};
static const ToneNote SOUND_ALARM[]     = {{2000,180},{0,100},{2000,180},{0,100},{2000,180},{0,0}};
static const ToneNote SOUND_ENROLL_OK[] = {{1047,80},{1319,80},{1568,120},{0,0}};
static const ToneNote SOUND_LOW_BATT[]  = {{600,150},{0,50},{600,150},{0,0}};

bool audioOK = false;

void initAudio() {
    i2s_config_t cfg = {
        .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate          = SAMPLE_RATE,
        .bits_per_sample      = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count        = 4,
        .dma_buf_len          = 256,
        .use_apll             = false,
        .tx_desc_auto_clear   = true
    };
    i2s_pin_config_t pins = {
        .bck_io_num   = I2S_BCLK_PIN,
        .ws_io_num    = I2S_LRCLK_PIN,
        .data_out_num = I2S_DATA_PIN,
        .data_in_num  = I2S_PIN_NO_CHANGE
    };
    if (i2s_driver_install(I2S_PORT, &cfg, 0, NULL) != ESP_OK) return;
    i2s_set_pin(I2S_PORT, &pins);
    audioOK = true;
    Serial.println("✅ I2S 音頻初始化完成");
}

void playNote(int hz, int durMs) {
    if (!audioOK || hz <= 0) { delay(durMs); return; }
    int total = SAMPLE_RATE * durMs / 1000;
    int16_t buf[512];
    int written = 0;
    while (written < total) {
        int chunk = min(256, total - written);
        int16_t stereo[512];
        for (int i = 0; i < chunk; i++) {
            int16_t s = (int16_t)(AUDIO_VOL * sinf(2.0f*M_PI*hz*(written+i)/SAMPLE_RATE));
            stereo[i*2] = stereo[i*2+1] = s;
        }
        size_t bw;
        i2s_write(I2S_PORT, stereo, chunk*4, &bw, portMAX_DELAY);
        written += chunk;
    }
}

void playSound(const ToneNote* notes) {
    for (int i = 0; notes[i].freq || notes[i].dur; i++)
        playNote(notes[i].freq, notes[i].dur);
}

// 非同步播放（不阻塞主迴圈）
struct _SndArg { const ToneNote* n; };
static void _sndTask(void* a) {
    playSound(((const _SndArg*)a)->n);
    delete (const _SndArg*)a;
    vTaskDelete(NULL);
}
void playSoundAsync(const ToneNote* notes) {
    auto* a = new _SndArg{notes};
    xTaskCreate(_sndTask, "snd", 3072, a, 1, NULL);
}
```

---

## 第二十九章：relay.h

```cpp
// src/relay.h
#pragma once
#include <Arduino.h>
#include "config.h"

bool _relayOpen = false;
unsigned long _relayOpenAt = 0;

void initRelay() {
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);  // 啟動時確保鎖緊
    Serial.println("✅ 繼電器初始化（鎖緊）");
}

void unlockDoor(unsigned long durMs = UNLOCK_DURATION_MS) {
    digitalWrite(RELAY_PIN, LOW);
    _relayOpen   = true;
    _relayOpenAt = millis();
    Serial.printf("🔓 開鎖 %lu ms\n", durMs);
}

void lockDoor() {
    digitalWrite(RELAY_PIN, HIGH);
    _relayOpen = false;
    Serial.println("🔒 已鎖門");
}

// 在 loop 中呼叫，處理自動鎖門計時
void updateRelay() {
    if (_relayOpen && millis() - _relayOpenAt >= UNLOCK_DURATION_MS)
        lockDoor();
}

bool isDoorOpen()          { return _relayOpen; }
int  unlockRemainingMs()   {
    if (!_relayOpen) return 0;
    long r = (long)UNLOCK_DURATION_MS - (long)(millis()-_relayOpenAt);
    return r > 0 ? (int)r : 0;
}
int  unlockRemainingSec()  { return unlockRemainingMs() / 1000 + 1; }
```

---

## 第三十章：wifi_mgr.h

```cpp
// src/wifi_mgr.h
#pragma once
#include <WiFi.h>
#include "config.h"

bool connectWiFi() {
    Serial.printf("連接 WiFi：%s ...", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    int tries = WIFI_TIMEOUT_SEC * 2;
    while (WiFi.status() != WL_CONNECTED && tries-- > 0) {
        delay(500); Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\n✅ 已連線 IP:%s RSSI:%ddBm\n",
            WiFi.localIP().toString().c_str(), WiFi.RSSI());
        return true;
    }
    Serial.println("\n⚠️ WiFi 失敗，離線運作");
    return false;
}

void syncTime() {
    if (WiFi.status() != WL_CONNECTED) return;
    configTime(TZ_OFFSET_SEC, DST_OFFSET_SEC, NTP_SERVER1, NTP_SERVER2);
    struct tm t; int r=0;
    while (!getLocalTime(&t) && r++<20) delay(500);
    if (getLocalTime(&t)) {
        char buf[32]; strftime(buf,sizeof(buf),"%Y/%m/%d %H:%M:%S",&t);
        Serial.printf("✅ NTP 同步：%s\n", buf);
    }
}

void maintainWiFi() {
    static unsigned long last = 0;
    if (millis()-last < 30000) return;
    last = millis();
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi 斷線，重連中...");
        WiFi.reconnect();
    }
}

String getCurrentTime(const char* fmt = "%H:%M") {
    struct tm t;
    if (!getLocalTime(&t)) return "--:--";
    char buf[32]; strftime(buf,sizeof(buf),fmt,&t);
    return String(buf);
}

String getCurrentDateTime() {
    return getCurrentTime("%Y/%m/%d %H:%M:%S");
}
```

---

## 第三十一章：weather.h

```cpp
// src/weather.h
#pragma once
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "config.h"

struct WeatherInfo {
    String description;
    float  temp      = 0;
    float  feelsLike = 0;
    int    humidity  = 0;
    float  windSpeed = 0;
    bool   rainToday = false;
    bool   valid     = false;
};

WeatherInfo getWeather() {
    WeatherInfo info;
    if (WiFi.status() != WL_CONNECTED) return info;

    String url = "http://api.openweathermap.org/data/2.5/weather?q=";
    url += String(OWM_CITY) + "," + String(OWM_COUNTRY);
    url += "&appid=" + String(OWM_API_KEY);
    url += "&units=metric&lang=" + String(OWM_LANG);

    HTTPClient http;
    http.setTimeout(6000);
    http.begin(url);
    int code = http.GET();

    if (code == HTTP_CODE_OK) {
        String body = http.getString();
        StaticJsonDocument<1024> doc;
        if (!deserializeJson(doc, body)) {
            info.description = doc["weather"][0]["description"].as<String>();
            info.temp        = doc["main"]["temp"].as<float>();
            info.feelsLike   = doc["main"]["feels_like"].as<float>();
            info.humidity    = doc["main"]["humidity"].as<int>();
            info.windSpeed   = doc["wind"]["speed"].as<float>();
            String m         = doc["weather"][0]["main"].as<String>();
            info.rainToday   = (m=="Rain"||m=="Drizzle"||m=="Thunderstorm");
            info.valid       = true;
            Serial.printf("天氣：%s %.1f°C\n", info.description.c_str(), info.temp);
        }
    }
    http.end();
    return info;
}

String getWeatherMessage(const WeatherInfo& w) {
    if (!w.valid) return "天氣資料無法取得";
    String msg = "";
    if (w.rainToday)       msg = "☔ 今天有雨，請帶傘！";
    else if (w.temp > 34)  msg = "☀️ " + String((int)w.temp) + "°C 炎熱，注意防曬";
    else if (w.temp < 14)  msg = "🧥 " + String((int)w.temp) + "°C 偏涼，記得加衣";
    else                   msg = "🌤 " + w.description + " " + String((int)w.temp) + "°C";
    if (w.windSpeed > 10)  msg += " 強風" + String((int)w.windSpeed) + "m/s";
    return msg;
}

String getWeatherShort(const WeatherInfo& w) {
    if (!w.valid) return "No data";
    char buf[20];
    snprintf(buf, sizeof(buf), "%.0fC %d%% %s",
             w.temp, w.humidity, w.rainToday ? "Rain" : "OK");
    return String(buf);
}
```

---

## 第三十二章：telegram_bot.h

```cpp
// src/telegram_bot.h
#pragma once
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "config.h"
#include "relay.h"
#include "battery.h"
#include "weather.h"
#include "face_recognition.h"
#include "face_database.h"
#include "fingerprint.h"

extern FaceRecognitionSystem faceSystem;
extern FaceDatabase          faceDB;
extern WeatherInfo           weatherCache;
extern BatteryMonitor        battery;
extern String                currentPassword;
extern int                   failCount;
extern SystemState           currentState;

WiFiClientSecure tgSecure;
UniversalTelegramBot bot(BOT_TOKEN, tgSecure);

// 待登錄人臉資訊
String pendingEnrollName = "";
bool   pendingEnroll     = false;
unsigned long pendingEnrollStart = 0;
int    pendingEnrollCount = 0;

void sendTelegramMessage(const String& text) {
    if (WiFi.status() != WL_CONNECTED) return;
    tgSecure.setInsecure();
    bot.sendMessage(CHAT_ID, text, "");
}

void sendTelegramPhoto(camera_fb_t* fb, const String& caption) {
    if (!fb || WiFi.status() != WL_CONNECTED) return;
    tgSecure.setInsecure();
    bot.sendPhotoByBinary(CHAT_ID,"image/jpeg",fb->len,fb->buf,caption,"");
}

void handleTelegramCommands() {
    if (WiFi.status() != WL_CONNECTED) return;
    tgSecure.setInsecure();
    int n = bot.getUpdates(bot.last_message_received + 1);

    for (int i = 0; i < n; i++) {
        String text   = bot.messages[i].text;  text.trim();
        String fromId = bot.messages[i].chat_id;

        // 安全驗證
        if (fromId != String(CHAT_ID)) {
            bot.sendMessage(fromId, "⛔ 未授權", "");
            sendTelegramMessage("⚠️ 未授權存取來自 ID:" + fromId);
            continue;
        }

        Serial.println("[TG] " + text);

        // ─── 門鎖控制 ───
        if (text == "/unlock") {
            unlockDoor(5000);
            sendTelegramMessage("✅ 遠端開鎖 5s\n" + getCurrentDateTime());
        }
        else if (text == "/status") {
            auto b = battery.getStatus(true);
            String s = "📊 門鎖狀態\n";
            s += "━━━━━━━━━━\n";
            s += "🔒 門：" + String(isDoorOpen()?"開啟":"鎖緊") + "\n";
            s += "📶 IP：" + WiFi.localIP().toString() + "\n";
            s += "⚡ 電：" + String(b.percentage) + "% " + String(b.voltage,2) + "V";
            if (b.charging) s += " ⚡充電中";
            s += "\n👤 人臉：" + String(faceSystem.getCount()) + " 筆\n";
            s += "⏱ 運行：" + String(millis()/60000) + " 分\n";
            s += "🕐 " + getCurrentDateTime();
            bot.sendMessage(CHAT_ID, s, "");
        }
        else if (text == "/alarm_off") {
            failCount    = 0;
            currentState = STATE_IDLE;
            sendTelegramMessage("🔕 警報已遠端解除");
        }
        else if (text == "/photo") {
            camera_fb_t* fb = esp_camera_fb_get();
            if (fb) {
                sendTelegramPhoto(fb, "📸 門口即時畫面\n" + getCurrentDateTime());
                esp_camera_fb_return(fb);
            } else {
                bot.sendMessage(CHAT_ID, "❌ 拍攝失敗", "");
            }
        }

        // ─── 天氣 / 電池 ───
        else if (text == "/weather") {
            WeatherInfo w = getWeather();
            String s  = "🌤 台北天氣\n━━━━━━━━━━\n";
            s += w.description + "\n";
            s += "🌡 " + String(w.temp,1) + "°C（體感" + String(w.feelsLike,1) + "°C）\n";
            s += "💧 " + String(w.humidity) + "%  💨 " + String(w.windSpeed,1) + "m/s\n";
            s += "\n" + getWeatherMessage(w);
            bot.sendMessage(CHAT_ID, s, "");
        }
        else if (text == "/battery") {
            auto b = battery.getStatus(true);
            String s = "🔋 電池狀態\n";
            s += "電量：" + String(b.percentage) + "%\n";
            s += "電壓：" + String(b.voltage,2) + "V\n";
            s += "狀態：" + String(b.charging ? "充電中 ⚡" : "使用中");
            if (b.lowBattery) s += "\n⚠️ 電量偏低！";
            bot.sendMessage(CHAT_ID, s, "");
        }

        // ─── 密碼修改 ───
        else if (text.startsWith("/set_password ")) {
            String np = text.substring(14); np.trim();
            if (np.length() >= 4 && np.length() <= (unsigned)MAX_PASSWORD_LEN) {
                currentPassword = np;
                bot.sendMessage(CHAT_ID, "✅ 密碼已更新", "");
            } else {
                bot.sendMessage(CHAT_ID,
                    "❌ 密碼需 4~" + String(MAX_PASSWORD_LEN) + " 位", "");
            }
        }

        // ─── 人臉管理 ───
        else if (text == "/face_list") {
            auto list = faceSystem.getList();
            if (list.empty()) {
                bot.sendMessage(CHAT_ID, "📭 無已登錄人臉", "");
            } else {
                String s = "👤 已登錄 " + String(list.size()) + " 筆\n━━━━━━━━━━\n";
                for (int j=0; j<(int)list.size(); j++)
                    s += String(j+1) + ". " + list[j] + "\n";
                bot.sendMessage(CHAT_ID, s, "");
            }
        }
        else if (text.startsWith("/face_enroll ")) {
            String name = text.substring(13); name.trim();
            if (name.isEmpty()) {
                bot.sendMessage(CHAT_ID, "用法：/face_enroll 王小明", "");
            } else if (faceSystem.getCount() >= MAX_FACE_COUNT) {
                bot.sendMessage(CHAT_ID, "❌ 人臉庫已滿，請先刪除", "");
            } else {
                pendingEnrollName  = name;
                pendingEnroll      = true;
                pendingEnrollStart = millis();
                pendingEnrollCount = 0;
                bot.sendMessage(CHAT_ID,
                    "📷 準備登錄：" + name +
                    "\n請在 30 秒內走到鏡頭前正面站立\n將自動拍攝 " +
                    String(FACE_ENROLL_SAMPLES) + " 張", "");
            }
        }
        else if (text.startsWith("/face_delete ")) {
            String name = text.substring(13); name.trim();
            bool ok1 = faceSystem.deleteByName(name);
            bool ok2 = faceDB.removeName(name);
            bot.sendMessage(CHAT_ID,
                (ok1||ok2) ? "✅ 已刪除："+name : "❌ 找不到："+name, "");
        }
        else if (text == "/face_deleteall") {
            bot.sendMessage(CHAT_ID, "⚠️ 確認清除？回覆 /face_deleteall_confirm", "");
        }
        else if (text == "/face_deleteall_confirm") {
            faceSystem.deleteAll();
            faceDB.removeAll();
            bot.sendMessage(CHAT_ID, "✅ 所有人臉已清除", "");
        }

        // ─── 指紋管理 ───
        else if (text.startsWith("/fp_enroll ")) {
            int id = text.substring(11).toInt();
            if (id < 1 || id > 127) {
                bot.sendMessage(CHAT_ID, "❌ ID 需為 1~127", "");
            } else {
                bot.sendMessage(CHAT_ID,
                    "👆 開始登錄指紋 #" + String(id) + "\n請按壓感測器...", "");
                bool ok = enrollFingerprint(id);
                bot.sendMessage(CHAT_ID,
                    ok ? "✅ 指紋 #"+String(id)+" 登錄成功"
                       : "❌ 登錄失敗，請重試", "");
            }
        }
        else if (text.startsWith("/fp_delete ")) {
            int id = text.substring(11).toInt();
            bot.sendMessage(CHAT_ID,
                deleteFingerprint(id) ? "✅ 指紋 #"+String(id)+" 已刪除"
                                      : "❌ 刪除失敗", "");
        }
        else if (text == "/fp_clear") {
            bot.sendMessage(CHAT_ID, "確認？回覆 /fp_clear_confirm", "");
        }
        else if (text == "/fp_clear_confirm") {
            clearAllFingerprints();
            bot.sendMessage(CHAT_ID, "✅ 所有指紋已清除", "");
        }

        // ─── 說明 ───
        else if (text == "/help" || text == "/start") {
            String h = "🔐 智慧門鎖指令\n━━━━━━━━━━\n";
            h += "🔑 *門鎖*\n/unlock /status /alarm_off /photo\n\n";
            h += "👤 *人臉*\n/face_list\n/face_enroll [名]\n/face_delete [名]\n/face_deleteall\n\n";
            h += "👆 *指紋*\n/fp_enroll [1-127]\n/fp_delete [ID]\n/fp_clear\n\n";
            h += "🔧 *設定*\n/set_password [新密碼]\n\n";
            h += "ℹ️ *資訊*\n/weather /battery";
            bot.sendMessage(CHAT_ID, h, "Markdown");
        }
        else {
            bot.sendMessage(CHAT_ID, "❓ 未知指令，輸入 /help", "");
        }
    }
}

// 在 loop 的人臉辨識段落中呼叫，處理 Telegram 觸發的人臉登錄
void checkPendingEnroll(camera_fb_t* fb) {
    if (!pendingEnroll || !fb) return;
    if (millis() - pendingEnrollStart > 30000) {
        pendingEnroll = false;
        sendTelegramMessage("⏰ 登錄逾時：" + pendingEnrollName);
        return;
    }
    bool ok = faceSystem.enrollOne(fb, pendingEnrollName);
    if (ok) {
        pendingEnrollCount++;
        if (pendingEnrollCount >= FACE_ENROLL_SAMPLES) {
            faceDB.saveName(pendingEnrollName);
            sendTelegramMessage("✅ 人臉登錄完成：" + pendingEnrollName);
            pendingEnroll = false;
        }
    }
}
```

---

## 第三十三章：main.cpp

```cpp
// src/main.cpp  —  智慧門鎖主程式
#include <Arduino.h>
#include <Wire.h>
#include <PCF8574.h>
#include "config.h"
#include "face_recognition.h"
#include "face_database.h"
#include "oled_ui.h"
#include "battery.h"
#include "keypad.h"
#include "fingerprint.h"
#include "audio.h"
#include "relay.h"
#include "wifi_mgr.h"
#include "weather.h"
#include "telegram_bot.h"

// ════ 全域物件 ════
FaceRecognitionSystem faceSystem;
FaceDatabase          faceDB;
OledUI                ui;
BatteryMonitor        battery;

PCF8574 keypadPCF(PCF_KEYPAD_ADDR, I2C_SDA_PIN, I2C_SCL_PIN);
PCF8574 statusPCF(PCF_STATUS_ADDR, I2C_SDA_PIN, I2C_SCL_PIN);

HardwareSerial     fpSerial(1);
Adafruit_Fingerprint finger(&fpSerial);

// ════ 執行時狀態（telegram_bot.h 的 extern 宣告對應這裡）════
SystemState  currentState   = STATE_IDLE;
String       currentPassword = DEFAULT_PASSWORD;
int          failCount       = 0;
WeatherInfo  weatherCache;

// 時間戳記
unsigned long lastWeatherUpdate = 0;
unsigned long lastBotPoll       = 0;
unsigned long lastBattCheck     = 0;
unsigned long lastOledUpdate    = 0;

String inputBuf = "";  // 鍵盤輸入緩衝

// ════ LED 輔助 ════
void setLED(bool green, bool red) {
    statusPCF.write(LED_GREEN_P, green ? LOW : HIGH);
    statusPCF.write(LED_RED_P,   red   ? LOW : HIGH);
}

// ════ 解鎖成功 ════
void successUnlock(const String& name, camera_fb_t* photo = nullptr) {
    Serial.printf("🔓 解鎖 [%s]\n", name.c_str());
    failCount    = 0;
    currentState = STATE_UNLOCKED;
    inputBuf     = "";

    unlockDoor(UNLOCK_DURATION_MS);
    playSoundAsync(SOUND_UNLOCK);
    setLED(true, false);

    String weather = WEATHER_NOTIFY_EN ? getWeatherMessage(weatherCache) : "";
    ui.showUnlocked(name, weather, UNLOCK_DURATION_MS / 1000);

    sendTelegramMessage("✅ 解鎖：" + name + "\n" +
                        getCurrentDateTime() + "\n" + weather);
}

// ════ 失敗處理 ════
void failedAttempt() {
    failCount++;
    playSoundAsync(SOUND_DENY);
    setLED(false, true);
    ui.showDenied(failCount, MAX_FAIL_ATTEMPTS);
    delay(1200);
    setLED(false, false);

    if (failCount >= MAX_FAIL_ATTEMPTS) {
        currentState = STATE_ALARM;
        sendTelegramMessage("🚨 警報！連續失敗 " +
                            String(MAX_FAIL_ATTEMPTS) + " 次\n" +
                            getCurrentDateTime());
    }
}

// ════ 鍵盤輸入處理 ════
void handleKeyInput(char key) {
    playSoundAsync(SOUND_BEEP);

    if (key == '*') {
        inputBuf = "";
        ui.showPasswordInput(0);
        return;
    }

    if (key == '#') {
        if (inputBuf == currentPassword) {
            successUnlock("Password");
        } else {
            failedAttempt();
        }
        inputBuf = "";
        return;
    }

    // A 鍵：進入人臉管理（需先輸入管理密碼）
    if (key == 'A') {
        if (inputBuf == ADMIN_PASSWORD) {
            currentState = STATE_FACE_MGMT;
            inputBuf     = "";
        } else {
            ui.showMessage("Access Denied", "Wrong admin pwd");
            delay(1500);
            inputBuf = "";
        }
        return;
    }

    if (isDigit(key) && inputBuf.length() < (unsigned)MAX_PASSWORD_LEN) {
        inputBuf += key;
        ui.showPasswordInput(inputBuf.length());
    }
}

// ════ 狀態：待機 ════
void handleIdle() {
    // OLED 每秒更新
    if (millis() - lastOledUpdate > 1000) {
        lastOledUpdate = millis();
        auto b = battery.getStatus();
        ui.showIdle(getCurrentTime(), weatherCache.temp,
                    weatherCache.rainToday, b.percentage, b.charging);
    }

    // 鍵盤掃描
    char key = scanKeypad(keypadPCF);
    if (key) handleKeyInput(key);

    // 指紋輪詢
    if (FINGERPRINT_EN && fpOK) {
        int fpId = verifyFingerprint();
        if (fpId > 0) {
            successUnlock("Fingerprint #" + String(fpId));
        } else if (fpId == -1) {
            failedAttempt();
        }
    }

    // 人臉辨識（限速）
    if (FACE_RECOGNITION_EN &&
        millis() - lastOledUpdate > FACE_SCAN_INTERVAL_MS) {
        camera_fb_t* fb = faceSystem.capture();
        if (fb) {
            // 若有待登錄任務，優先處理
            if (pendingEnroll) {
                checkPendingEnroll(fb);
                ui.showEnrollProgress(pendingEnrollName,
                    pendingEnrollCount, FACE_ENROLL_SAMPLES);
            } else {
                ui.showVerifying("Face Recognition");
                auto res = faceSystem.process(fb);

                if (res.recognized) {
                    successUnlock(res.name, fb);
                } else if (res.face_detected && STRANGER_ALERT_EN) {
                    static unsigned long lastAlert = 0;
                    if (millis() - lastAlert > 30000) {  // 30 秒內只報警一次
                        lastAlert = millis();
                        Serial.println("⚠️ 陌生人偵測");
                        String cap = "⚠️ 陌生人！\n" + getCurrentDateTime();
                        sendTelegramPhoto(fb, cap);
                    }
                }
            }
            faceSystem.returnFrame(fb);
        }
    }
}

// ════ 狀態：已解鎖 ════
void handleUnlocked() {
    updateRelay();  // 計時自動鎖門

    static int lastSec = -1;
    int sec = unlockRemainingSec();
    if (sec != lastSec) {
        lastSec = sec;
        ui.showUnlocked("Welcome!", getWeatherShort(weatherCache), sec);
    }

    if (!isDoorOpen()) {
        currentState = STATE_IDLE;
        setLED(false, false);
        lastSec = -1;
    }
}

// ════ 狀態：警報 ════
void handleAlarm() {
    ui.showAlarm();
    static unsigned long lastBeep = 0;
    if (millis() - lastBeep > 2000) {
        lastBeep = millis();
        playSoundAsync(SOUND_ALARM);
        setLED(false, (millis()/350)%2);
    }

    char key = scanKeypad(keypadPCF);
    if (!key) return;

    if (key == '*') {
        inputBuf = "";
    } else if (key == '#') {
        if (inputBuf == currentPassword || inputBuf == ADMIN_PASSWORD) {
            failCount    = 0;
            currentState = STATE_IDLE;
            setLED(false, false);
            sendTelegramMessage("ℹ️ 警報已由本地解除");
        }
        inputBuf = "";
    } else if (isDigit(key)) {
        inputBuf += key;
    }
}

// ════ 狀態：人臉管理選單 ════
void handleFaceMgmt() {
    ui.showFaceMenu(faceSystem.getCount());

    char key = waitForKey(keypadPCF, 15000);

    if (key == '1') {
        ui.showMessage("Add Face", "Use Telegram:\n/face_enroll [name]");
        sendTelegramMessage("📷 請使用 Telegram 指令登錄人臉：\n/face_enroll 姓名");
        delay(3000);
    } else if (key == '2') {
        auto list = faceSystem.getList();
        ui.showList("Faces (del via TG)", list);
        sendTelegramMessage("請使用 Telegram 刪除：\n/face_delete 姓名\n或 /face_list 查看清單");
        delay(4000);
    } else if (key == '3') {
        auto list = faceSystem.getList();
        if (list.empty()) ui.showMessage("No faces", "Database empty");
        else              ui.showList("Enrolled Faces", list);
        delay(4000);
    } else if (key == '4') {
        ui.showMessage("Confirm?", "Press # to delete ALL");
        char c = waitForKey(keypadPCF, 5000);
        if (c == '#') {
            faceSystem.deleteAll();
            faceDB.removeAll();
            ui.showMessage("Done", "All faces deleted");
            playSoundAsync(SOUND_DENY);
            delay(2000);
        }
    }

    if (key == '*' || key == 0) {
        currentState = STATE_IDLE;
    }
}

// ════ setup ════
void setup() {
    Serial.begin(115200);
    delay(2000);
    Serial.println("╔════════════════════════════╗");
    Serial.println("║   智慧門鎖 V2  啟動中...  ║");
    Serial.println("╚════════════════════════════╝");

    // 1. I2C + OLED
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    ui.begin();
    ui.showMessage("Booting...", "Init hardware");

    // 2. 基礎硬體
    initRelay();
    keypadPCF.begin();
    statusPCF.begin();
    setLED(false, false);
    battery.begin();

    // 3. 鏡頭與人臉辨識
    ui.showMessage("Booting...", "Camera init");
    if (!faceSystem.initCamera()) {
        ui.showMessage("ERROR", "Camera failed!");
        delay(5000);
    }

    // 4. SPIFFS
    ui.showMessage("Booting...", "Load face DB");
    faceDB.begin();
    // 提示：若 SPIFFS 中有存名稱但辨識器無向量，提示重新登錄
    auto savedNames = faceDB.loadNames();
    if (!savedNames.empty() && faceSystem.getCount() == 0) {
        Serial.println("⚠️ 偵測到已存名稱但辨識器空白，請重新登錄人臉");
        sendTelegramMessage("⚠️ 人臉資料庫需重建，請重新使用 /face_enroll 登錄");
    }

    // 5. 指紋
    ui.showMessage("Booting...", "Fingerprint");
    initFingerprint();

    // 6. 音頻
    initAudio();

    // 7. WiFi + NTP
    ui.showMessage("Booting...", "Connect WiFi");
    bool wifiOK = connectWiFi();
    if (wifiOK) {
        syncTime();
        weatherCache     = getWeather();
        lastWeatherUpdate = millis();
    }

    // 8. 啟動完成
    playSoundAsync(SOUND_STARTUP);
    setLED(true, false);

    if (wifiOK) {
        sendTelegramMessage(
            "🔐 智慧門鎖已啟動\n"
            "IP: " + WiFi.localIP().toString() + "\n"
            "人臉: " + String(faceSystem.getCount()) + " 筆\n"
            "電量: " + battery.toShortString() + "\n" +
            getCurrentDateTime()
        );
    }

    Serial.println("✅ 系統啟動完成，進入待機");
    currentState = STATE_IDLE;
}

// ════ loop ════
void loop() {
    // 維持 WiFi 連線
    maintainWiFi();

    // Telegram 輪詢（每 3 秒）
    if (millis() - lastBotPoll > BOT_POLL_MS) {
        lastBotPoll = millis();
        handleTelegramCommands();
    }

    // 天氣更新
    if (millis() - lastWeatherUpdate > WEATHER_UPDATE_MS) {
        lastWeatherUpdate = millis();
        weatherCache = getWeather();
    }

    // 電池監控（每 30 秒）
    if (millis() - lastBattCheck > 30000) {
        lastBattCheck = millis();
        auto b = battery.getStatus(true);
        if (b.lowBattery) {
            static bool alerted = false;
            if (!alerted) {
                alerted = true;
                playSoundAsync(SOUND_LOW_BATT);
                sendTelegramMessage("⚡ 電量不足：" +
                                    String(b.percentage) + "%，請充電");
            }
        }
        // 嚴重低電：降頻省電
        if (b.percentage < CRITICAL_BATTERY_PCT) {
            setCpuFrequencyMhz(80);
        }
    }

    // 狀態機
    switch (currentState) {
        case STATE_IDLE:      handleIdle();      break;
        case STATE_UNLOCKED:  handleUnlocked();  break;
        case STATE_ALARM:     handleAlarm();     break;
        case STATE_FACE_MGMT: handleFaceMgmt();  break;
    }
}
```

---

# 第七部分：設定、測試與維護

---

## 第三十四章：platformio.ini 與分割表

### platformio.ini（完整版）

```ini
; ═══════════════════════════════════════════════
; 智慧門鎖 V2 — PlatformIO 設定
; ═══════════════════════════════════════════════

[env:seeed_xiao_esp32s3]
platform  = espressif32 @ ~6.5.0
board     = seeed_xiao_esp32s3
framework = arduino

monitor_speed = 115200
upload_speed  = 921600

; 自訂分割表（支援 OTA + 2MB SPIFFS）
board_build.partitions          = partitions_smartlock.csv

; 啟用 OPI PSRAM（人臉辨識必須）
board_build.arduino.memory_type = qio_opi

build_flags =
    -DBOARD_HAS_PSRAM
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DCORE_DEBUG_LEVEL=0
    -mfix-esp32-psram-cache-issue
    ; esp-face 框架 include 路徑
    -I${platformio.packages_dir}/framework-arduinoespressif32/tools/sdk/esp32s3/include/esp-face/include/typedef
    -I${platformio.packages_dir}/framework-arduinoespressif32/tools/sdk/esp32s3/include/esp-face/include/math
    -I${platformio.packages_dir}/framework-arduinoespressif32/tools/sdk/esp32s3/include/esp-face/include/image
    -I${platformio.packages_dir}/framework-arduinoespressif32/tools/sdk/esp32s3/include/esp-face/include/model_zoo
    -I${platformio.packages_dir}/framework-arduinoespressif32/tools/sdk/esp32s3/include/esp-face/include/face_detection
    -I${platformio.packages_dir}/framework-arduinoespressif32/tools/sdk/esp32s3/include/esp-face/include/face_recognition

lib_deps =
    espressif/esp32-camera @ ^2.0.4
    adafruit/Adafruit SSD1306 @ ^2.5.7
    adafruit/Adafruit GFX Library @ ^1.11.9
    adafruit/Adafruit Fingerprint Sensor Library @ ^2.1.0
    renzo-mischianti/PCF8574 @ ^2.3.6
    bblanchon/ArduinoJson @ ^6.21.3
    knolleary/PubSubClient @ ^2.8.0
    schreibfaul1/ESP32-audioI2S @ ^2.0.7
    witnessmenow/Universal-Arduino-Telegram-Bot @ ^1.3.0
```

### partitions_smartlock.csv

```csv
# Name,   Type, SubType,  Offset,   Size,    Flags
nvs,      data, nvs,      0x9000,   0x5000,
otadata,  data, ota,      0xe000,   0x2000,
app0,     app,  ota_0,    0x10000,  0x300000,
app1,     app,  ota_1,    0x310000, 0x300000,
spiffs,   data, spiffs,   0x610000, 0x1F0000,
```

> 分割說明：app0/app1 各 3MB（支援 OTA 更新），spiffs 約 2MB（存人臉名稱與設定）

---

## 第三十五章：燒錄與完整測試清單

### 35.1 燒錄步驟

```
Step 1. 用 USB-C 連接 XIAO ESP32 S3 至電腦
Step 2. VS Code 左下角確認 COM Port（如 COM8 / /dev/ttyACM0）
Step 3. 按 Ctrl+Alt+U 上傳（或點底部工具列 → 箭頭圖示）
Step 4. 若出現 "A fatal error: Failed to connect"：
        按住開發板 Boot 按鈕 → 再按上傳 → 看到 "Connecting..."後放開
Step 5. 上傳完成後按 Ctrl+Alt+S 開啟序列埠監視器（115200 baud）
Step 6. 第一次燒錄後，上傳 SPIFFS：
        PlatformIO 圖示 → Project Tasks → Platform → Upload Filesystem Image
```

### 35.2 完整測試清單（依序完成，每項通過再繼續）

#### ▶ 階段 A：硬體基礎

```
□ A1  驗證程式：序列埠顯示 CPU 240MHz、PSRAM 8MB、Flash 8MB
□ A2  I2C 掃描：找到 0x20、0x21、0x3C 共三個裝置
□ A3  OLED：顯示文字，更新計時器數字，無花屏
□ A4  鍵盤：全部 16 個按鍵正確顯示字元（1~9,0,*,#,A~D）
□ A5  AS608：verifyPassword() = true；登錄 ID=1 成功；驗證成功；陌生手指回 -1
□ A6  MAX98357：喇叭發出清晰 SOUND_UNLOCK 音效
□ A7  繼電器：聽到繼電器動作聲；電磁鎖正確開/鎖
□ A8  電池 ADC：讀值換算電壓合理（3.3V~4.2V）；充電時 CHRG = LOW
□ A9  鏡頭：initCamera 成功；RGB565 240×240 每幀 >100KB
```

#### ▶ 階段 B：人臉辨識

```
□ B1  esp-face 編譯通過（無 "file not found" 錯誤）
□ B2  人臉偵測 Stage 1：站到鏡頭前印出 "找到候選框"
□ B3  人臉偵測 Stage 2：印出關鍵點座標（5組，均在 0~240 範圍內）
□ B4  人臉登錄（Telegram）：
        傳 /face_enroll 王小明 → 走到鏡頭前 → 等待 5 張完成
□ B5  人臉辨識：走到鏡頭前 → 顯示 "Welcome! 王小明" → 繼電器開門
□ B6  陌生人偵測：不同人走到鏡頭前 → Telegram 收到照片警報
□ B7  人臉刪除：傳 /face_delete 王小明 → 再次驗證無法解鎖
□ B8  SPIFFS 持久化：重啟後傳 /face_list 確認人臉仍在
```

#### ▶ 階段 C：OLED UI

```
□ C1  待機畫面：時間每秒更新、電量條正確、天氣資訊顯示
□ C2  密碼輸入畫面：按數字鍵顯示對應數量 "*"，按 * 歸零
□ C3  驗證旋轉動畫：走到鏡頭前顯示轉動動畫
□ C4  解鎖成功畫面：顯示姓名、天氣提示、倒數秒數
□ C5  驗證失敗畫面：顯示失敗次數
□ C6  警報畫面：紅燈閃爍、文字閃爍
□ C7  管理選單：輸入 ADMIN_PASSWORD + # + A 進入選單
□ C8  低電量：臨時改 LOW_BATTERY_PCT=99 觸發警告畫面，測完改回 20
```

#### ▶ 階段 D：雲端功能

```
□ D1  WiFi：連線成功，序列埠顯示 IP 位址
□ D2  NTP：時間同步，顯示正確台灣時間（UTC+8）
□ D3  天氣：傳 /weather，收到含台北溫度的回覆
□ D4  Telegram 開鎖：傳 /unlock → 繼電器動作 5 秒
□ D5  Telegram 狀態：傳 /status → 收到電量、IP、人臉數
□ D6  未授權拒絕：用另一個帳號傳指令 → 收到警告通知
□ D7  WiFi 重連：關閉路由器 30 秒再開 → ESP32 自動重連
□ D8  電量低通知：電量低於 20% 時 Telegram 收到提醒
```

#### ▶ 階段 E：整合測試

```
□ E1  三種解鎖全部正常
        □ 人臉  → OLED 顯示姓名、Telegram 通知、繼電器動作
        □ 指紋  → OLED 顯示 "Fingerprint #N"、通知
        □ 密碼  → OLED 顯示 "Welcome!"、通知
□ E2  連續失敗 → 警報 → 解除
        輸入 5 次錯誤密碼 → 警報觸發 → 用正確密碼或 /alarm_off 解除
□ E3  出門天氣提醒：解鎖後 OLED 顯示天氣（若下雨顯示帶傘）
□ E4  電池備援：拔除 USB → 系統繼續正常運作
□ E5  24 小時穩定性：長跑 24 小時無崩潰（無 Guru Meditation Error）
```

---

## 第三十六章：日常操作說明

### 36.1 開門方式

| 方式 | 操作步驟 | 所需時間 |
|------|---------|---------|
| **人臉辨識** | 直接面對鏡頭站立，距離 30~60cm | ~0.5~1 秒 |
| **指紋** | 按壓 AS608 指紋感測器表面 | ~0.5 秒 |
| **密碼** | 輸入 4~8 位數字 → 按 `#` 確認 | 依輸入速度 |
| **Telegram** | 傳送 `/unlock` 給 Bot | ~2~5 秒（含網路延遲）|

### 36.2 按鍵功能速查

| 按鍵 | 功能 |
|------|------|
| `1`~`9`, `0` | 輸入密碼數字 |
| `#` | 確認密碼 |
| `*` | 清除已輸入的密碼 |
| `[ADMIN密碼]` + `#` + `A` | 進入管理選單 |
| 管理選單中 `1` | 提示使用 Telegram 新增人臉 |
| 管理選單中 `2` | 顯示人臉清單（刪除請用 Telegram）|
| 管理選單中 `3` | 列出所有已登錄人臉 |
| 管理選單中 `4` | 清除所有人臉（按 `#` 二次確認）|
| 管理選單中 `*` | 返回待機 |
| 警報時 `[密碼]` + `#` | 解除警報 |

### 36.3 Telegram 指令完整表

| 類別 | 指令 | 功能 |
|------|------|------|
| 門鎖 | `/unlock` | 遠端開鎖 5 秒 |
| 門鎖 | `/status` | 查詢系統狀態（IP/電量/人臉數）|
| 門鎖 | `/alarm_off` | 遠端解除警報 |
| 門鎖 | `/photo` | 拍攝門口即時畫面 |
| 人臉 | `/face_list` | 列出所有已登錄人臉 |
| 人臉 | `/face_enroll [姓名]` | 觸發人臉登錄（30 秒內走到鏡頭前）|
| 人臉 | `/face_delete [姓名]` | 刪除指定人臉 |
| 人臉 | `/face_deleteall` | 準備清除所有人臉（需二次確認）|
| 人臉 | `/face_deleteall_confirm` | 確認清除所有人臉 |
| 指紋 | `/fp_enroll [1~127]` | 觸發指紋登錄（按壓感測器）|
| 指紋 | `/fp_delete [ID]` | 刪除指定 ID 指紋 |
| 指紋 | `/fp_clear` | 準備清除所有指紋 |
| 指紋 | `/fp_clear_confirm` | 確認清除所有指紋 |
| 設定 | `/set_password [新密碼]` | 修改解鎖密碼（4~8 位數字）|
| 資訊 | `/weather` | 查詢目前天氣 |
| 資訊 | `/battery` | 查詢電池狀態 |
| 說明 | `/help` | 顯示所有指令說明 |

---

## 第三十七章：常見問題排查

### Q1：PSRAM 找不到 / 鏡頭初始化失敗 0x105

```
原因：platformio.ini 中未正確啟用 OPI PSRAM
解決：
  確認 platformio.ini 有這一行：
  board_build.arduino.memory_type = qio_opi

  並且 build_flags 有：
  -DBOARD_HAS_PSRAM
  -mfix-esp32-psram-cache-issue

  儲存後執行 PlatformIO Terminal：
  > pio run --target clean
  > pio run
```

### Q2：編譯失敗 "human_face_detect_msr01.hpp: No such file"

```
原因：esp-face 的 include 路徑設定有誤
解決：
  1. 先確認 ESP32 Arduino SDK 已安裝：
     PlatformIO Terminal > pio platform update espressif32

  2. 查找實際路徑（Windows 範例）：
     %USERPROFILE%\.platformio\packages\framework-arduinoespressif32\
     tools\sdk\esp32s3\include\esp-face\include\

     若路徑不存在，代表 SDK 版本過舊，更新後重試

  3. Mac/Linux：
     ~/.platformio/packages/framework-arduinoespressif32/
```

### Q3：I2C 只找到部分裝置

```
逐一排查：
  □ 確認 PCF8574 #2 的 A0 接 VCC（不是 GND）
  □ 用三用電表確認所有模組的 VCC 有 3.3V
  □ SDA/SCL 線路是否有共同上拉電阻（4.7kΩ 到 3.3V）
  □ 線路是否過長（超過 30cm）→ 換 2.2kΩ 上拉
  □ 確認 Wire.begin(5, 6) 在所有 I2C 操作之前呼叫
```

### Q4：AS608 無法連線（verifyPassword 回傳 false）

```
檢查順序：
  1. AS608 的 VCC 確認接 3.3V（不是 5V！）
  2. TX/RX 是否交叉：AS608 TXD → ESP32 GPIO44（RX）
                     AS608 RXD → ESP32 GPIO43（TX）
  3. 初始化順序確認：
     fpSerial.begin(57600, SERIAL_8N1, 44, 43); // RX腳, TX腳
     delay(200);
     finger.begin(57600);
  4. 某些 AS608 預設密碼非 0x00000000：
     嘗試 finger.verifyPassword(0xFFFFFFFF)
```

### Q5：指紋辨識信心過低（< 40）

```
原因：登錄品質差或手指有汗/傷口
解決：
  1. 清除現有指紋：Serial 輸入 C 或傳 /fp_clear_confirm
  2. 手指擦乾後重新登錄
  3. 登錄時放穩，確保指紋中心對準感測器
  4. 若皮膚乾燥，先輕微呼氣到手指上再登錄
  5. 提高信心門檻（目前 40）可調到 30 增加靈敏度：
     在 fingerprint.h 修改 if (finger.confidence < 40)
```

### Q6：人臉辨識對同一個人有時通過有時失敗

```
原因：光線條件不穩定、特徵向量樣本不足
解決：
  1. 增加登錄樣本：修改 FACE_ENROLL_SAMPLES 從 5 改為 8~10
  2. 登錄時多角度：正面、輕微左轉、右轉各拍幾張
  3. 確保鏡頭前光線充足且均勻（正面補光最重要）
  4. 鏡頭玻璃有指紋或霧氣 → 清潔鏡頭
  5. 調低 Stage 1 偵測門檻（更靈敏但可能誤報）：
     detector1(0.2F, 0.3F, 10, 0.2F)  ← 第1和第4個參數調低
```

### Q7：Telegram 沒有回應或延遲很久

```
排查步驟：
  1. 確認 Bot Token 格式正確（純英數+冒號+字母）
  2. 確認 Chat ID 是純數字（個人帳號的 ID 可能有負號，群組 ID 有負號）
  3. tgSecure.setInsecure() 必須在每次 bot 操作前呼叫
  4. BOT_POLL_MS 不要低於 1000ms（Telegram 有速率限制）
  5. 用手機測試能否 ping api.telegram.org
  6. 若在中國大陸地區，需要使用 VPN 或代理
```

### Q8：電池電壓讀值不準確

```
校準方法：
  1. 用三用電表量 B+ 和 GND 之間的電壓（設電壓量程）
  2. 比較 Serial 輸出的 "電池電壓" 值
  3. 若程式讀值比實際高：分壓倍數從 2.0 改小（改 1.9 或 1.8）
  4. 若程式讀值比實際低：分壓倍數改大（改 2.1 或 2.2）
  
  更精確做法（使用 esp_adc_cal）：
  在 battery.h 中加入：
  #include <esp_adc_cal.h>
  esp_adc_cal_characteristics_t chars;
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_11db,
                           ADC_WIDTH_BIT_12, 1100, &chars);
  // 讀取時：
  uint32_t mv = esp_adc_cal_raw_to_voltage(raw, &chars);
  float battV = mv / 1000.0f * 2.0f;
```

### Q9：喇叭出現嗡嗡雜音

```
解決：
  1. MAX98357 的 VIN 確認接 5V（若接 3.3V 會有更多雜音）
  2. 在 VIN 和 GND 之間加一個 100μF 電容（去耦）
  3. 音訊線路遠離高電流線路（12V 電源線、繼電器控制線）
  4. 確認 GND 是同一個共地點（多點接地可能造成地迴路噪音）
  5. 喇叭線不要與電源線平行走線
```

### Q10：系統運作一段時間後崩潰（Guru Meditation Error）

```
常見原因和解決：
  1. camera_fb_t 未歸還（記憶體洩漏）
     → 確認每個 esp_camera_fb_get() 都有對應 esp_camera_fb_return()
     
  2. Stack overflow（任務堆疊溢出）
     → 增加 xTaskCreate 的 stack 大小（第5個參數），如 3072 → 4096
     
  3. Heap 耗盡
     → 每分鐘印出 ESP.getFreeHeap()，若持續下降代表有洩漏
     
  4. Watchdog timeout（某個函式執行太久）
     → 確認 loop() 中沒有 delay(超過 5000ms) 的阻塞呼叫
     → 人臉辨識改用非同步方式或縮短每幀處理時間
```

---

## 第三十八章：附錄：快速參考表

### 38.1 接線速查表

| 功能 | XIAO 腳位 | GPIO | 對接模組 |
|------|----------|------|---------|
| I2C SDA | D4 | GPIO5 | OLED / PCF8574 #1 / #2 |
| I2C SCL | D5 | GPIO6 | OLED / PCF8574 #1 / #2 |
| AS608 TX→ | D6 | GPIO43 | AS608 RXD |
| AS608 RX← | D7 | GPIO44 | AS608 TXD |
| I2S BCLK | D1 | GPIO2 | MAX98357 BCLK |
| I2S LRCLK | D2 | GPIO3 | MAX98357 LRC |
| I2S DATA | D3 | GPIO4 | MAX98357 DIN |
| 繼電器 | D0 | GPIO1 | 繼電器 IN |
| 電池 ADC | D9 | GPIO8 | 分壓中點 |
| 充電狀態 | D8 | GPIO7 | TP4056 CHRG |
| **HC-SR04 ECHO** | **D10** | **GPIO9** | **HC-SR04 ECHO（經 1kΩ/2kΩ 分壓）** |
| HC-SR04 TRIG | — | — | PCF8574 #2 P3（I2C 輸出）|
| 5V | 5V | — | MAX98357/繼電器 VCC |
| GND | GND | — | 所有模組 GND |

> ⚠️ **電磁鎖 12V 獨立電源，嚴禁接 ESP32 的任何腳位！**

### 38.2 PCF8574 地址與腳位對應

| 模組 | 地址 | A0 | A1 | A2 | 功能 |
|------|------|----|----|-----|------|
| PCF8574 #1 | 0x20 | GND | GND | GND | 鍵盤 Row/Col |
| PCF8574 #2 | 0x21 | VCC | GND | GND | LED / 蜂鳴器 |

**PCF8574 #1 腳位（鍵盤）**

| P0 | P1 | P2 | P3 | P4 | P5 | P6 | P7 |
|----|----|----|----|----|----|----|-----|
| R1 | R2 | R3 | R4 | C1 | C2 | C3 | C4 |

**PCF8574 #2 腳位（狀態）**

| P0 | P1 | P2 | **P3** | P4~P7 |
|----|----|----|--------|--------|
| 綠色 LED | 紅色 LED | 蜂鳴器 | **HC-SR04 TRIG** | 預留 |

### 38.3 I2C 裝置地址

| 裝置 | 地址 |
|------|------|
| PCF8574 #1（鍵盤）| 0x20 |
| PCF8574 #2（LED）| 0x21 |
| SSD1306 OLED | 0x3C（部分模組為 0x3D）|

### 38.4 音效清單

| 常數名 | 音效描述 | 觸發時機 |
|--------|---------|---------|
| SOUND_STARTUP | 上升三音 | 系統啟動完成 |
| SOUND_UNLOCK | 歡快三音 | 解鎖成功 |
| SOUND_DENY | 低沉下降二音 | 驗證失敗 |
| SOUND_BEEP | 短嗶一聲 | 按鍵反饋 |
| SOUND_ALARM | 急促三連嗶 | 警報 |
| SOUND_ENROLL_OK | 上升三音 | 人臉登錄成功 |
| SOUND_LOW_BATT | 低頻二短嗶 | 電量不足 |

### 38.5 系統狀態機

```
              ┌─────────────────────────────┐
              │          STATE_IDLE          │ ← 預設待機
              │  ・OLED 顯示時間/天氣/電量   │
              │  ・掃描鍵盤/指紋/人臉        │
              └────────────┬────────────────┘
                           │
          ┌────────────────┼────────────────┐
          │ 驗證成功       │ 連續 5 次失敗  │ 管理密碼+A
          ▼                ▼                ▼
  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐
  │STATE_UNLOCKED│  │ STATE_ALARM  │  │STATE_FACE_MGMT│
  │ 開鎖 5 秒   │  │ 警報閃爍     │  │ 管理選單     │
  │ 倒數顯示    │  │ 蜂鳴警報     │  │ 新增/刪除人臉│
  └──────┬───────┘  └──────┬───────┘  └──────┬───────┘
         │                 │                  │
         │ 計時結束        │ 密碼/TG 解除     │ * 或逾時
         └────────────────►└──────────────────┘
                           ▼
                    回到 STATE_IDLE
```

### 38.6 開發流程回顧

完成本專題後，你已掌握以下嵌入式開發技能：

| 技能 | 對應章節 |
|------|---------|
| I2C 匯流排與多裝置管理 | 第九章、PCF8574 |
| SPI/I2S 數位音頻協定 | 第十三章、audio.h |
| UART 序列通訊 | 第十二章、fingerprint.h |
| 矩陣鍵盤掃描演算法 | 第十一章、keypad.h |
| ADC 電壓量測與分壓電路 | 第十六章、battery.h |
| 神經網路模型部署（TFLite）| 第十五章、face_recognition.h |
| SPIFFS 檔案系統 | 第二十三章、face_database.h |
| FreeRTOS 任務（Task）| audio.h 非同步播放 |
| HTTP REST API 呼叫 | 第十八章、weather.h |
| SSL/TLS 安全通訊 | 第十九章、telegram_bot.h |
| 狀態機設計模式 | main.cpp |
| 超聲波測距原理 | U 章節、ultrasonic.h |
| 電壓準位轉換（分壓電路）| U2 接線圖 |
| 繼電器安全控制高電壓 | 第十四章、relay.h |

---

*文件版本：V2.0 完整版 ｜ 硬體：Seeed XIAO ESP32 S3 Sense ｜ 開發環境：PlatformIO + Arduino Framework + esp-face*
