# 🔐 智慧門鎖 V2（PIR 人體感測器版）

一個基於 ESP32-S3 的多功能智慧門鎖系統，整合人臉辨識、指紋辨識、密碼輸入、OLED 顯示、電池備用電源、Telegram 遠端控制和紅外線近接喚醒功能。

## 📋 功能特色

- **紅外線近接喚醒** - HC-SR501 PIR 人體感測器，偵測門外有人時自動喚醒，無人到離開後進入休眠模式，大幅省電
- **門內天氣播報** - 偵測到門內有人移動時，自動播報天氣資訊（5分鐘冷卻避免重複）
- **本地人臉辨識** - 使用 MTCNN + MobileFaceNet（esp-face 框架），不需網路即可辨識
- **人臉資料庫管理** - 使用 SPIFFS 持久化儲存，支援新增/刪除，重啟後保留
- **指紋辨識** - 支援 AS608 指紋模組，最多可儲存 127 枚指紋
- **密碼開鎖** - 4×4 矩陣鍵盤輸入密碼
- **OLED 顯示** - 128×64 SSD1306 螢幕，顯示時間、天氣、電量等資訊
- **電池備用電源** - 支援 18650 鋰電池，斷電後繼續運作
- **Telegram Bot** - 遠端控制、狀態查詢、人臉管理（休眠期間仍可遠端開鎖）
- **天氣提醒** - 開鎖時顯示天氣資訊和出門建議
- **I2S 音頻** - 使用 MAX98357 播放提示音

## 🛠️ 硬體需求

### 主要控制器
- **Seeed XIAO ESP32 S3 Sense**（內建 OV2640 鏡頭）

### 周邊模組
| 模組 | 型號 | 數量 | 備註 |
|------|------|------|------|
| OLED 顯示器 | SSD1306 128×64 I2C | 1 | |
| 指紋模組 | AS608 | 1 | UART 介面 |
| 鍵盤 | 4×4 矩陣鍵盤 | 1 | |
| I2C 擴充板 | PCF8574 | 2 | 一個用於鍵盤，一個用於 LED + 門內 PIR |
| 音頻擴大機 | MAX98357 | 1 | I2S 介面 |
| 繼電器模組 | 5V 繼電器 | 1 | 控制電磁鎖 |
| 紅外線感測 | HC-SR501 | 2 | 一個門外、一個門內 |
| 鋰電池 | 18650 3.7V | 1~2 | 備用電源 |
| 充電模組 | TP4056 Type-C | 1 | 鋰電池充電 |
| UPS 模組 | IP5306 | 1 | 不斷電系統 |

### 腳位配置（PIR 版）

| 功能 | XIAO ESP32 S3 腳位 | GPIO | 連接 |
|------|---------------------|------|------|
| I2C SDA | D4 | GPIO5 | OLED / PCF8574 #1 / #2 |
| I2C SCL | D5 | GPIO6 | OLED / PCF8574 #1 / #2 |
| AS608 TX→ | D6 | GPIO43 | AS608 RXD |
| AS608 RX← | D7 | GPIO44 | AS608 TXD |
| I2S BCLK | D1 | GPIO2 | MAX98357 BCLK |
| I2S LRC | D2 | GPIO3 | MAX98357 LRC |
| I2S DIN | D3 | GPIO4 | MAX98357 DIN |
| 繼電器 | D0 | GPIO1 | 繼電器 IN |
| 電池 ADC | D9 | GPIO8 | 分壓中點 |
| 充電狀態 | D8 | GPIO7 | TP4056 CHRG |
| 門外 PIR | D10 | GPIO9 | 門外 HC-SR501 OUT |

### PCF8574 #2（0x21）腳位分配

| 腳位 | 功能 |
|------|------|
| P0 | 綠色 LED |
| P1 | 紅色 LED |
| P2 | 蜂鳴器（選配）|
| P3 | 門內 PIR 輸入 |

### 硬體接線說明

#### 門外 PIR（喚醒系統）
- VCC → 3.3V
- GND → GND
- OUT → GPIO9 (D10)

#### 門內 PIR（天氣播報）
- VCC → 3.3V
- GND → GND
- OUT → PCF8574 #2 P3

## 📁 專案結構

```
SmartLock_V2/
├── platformio.ini           # PlatformIO 設定檔
├── partitions_smartlock.csv # Flash 分割表
├── README.md                # 本檔案
├── src/
│   ├── main.cpp             # 主程式（含狀態機、PIR 喚醒）
│   ├── config.h             # 所有設定（WiFi、密碼、PIR）
│   ├── pir_sensor.h         # HC-SR501 PIR 人體感測器（新）
│   ├── face_recognition.h   # 人臉辨識核心（MTCNN + MobileFaceNet）
│   ├── face_database.h      # 人臉資料庫（SPIFFS 持久化）
│   ├── face_manager.h       # 人臉管理輯
│   ├── oled_ui.h            # OLED 多頁面 UI
│   ├── battery.h            # 電池監控
│   ├── keypad.h             # 鍵盤掃描（PCF8574）
│   ├── fingerprint.h        # AS608 指紋模組
│   ├── audio.h              # I2S 音頻播放
│   ├── relay.h              # 繼電器控制
│   ├── wifi_mgr.h           # WiFi 管理
│   ├── telegram_bot.h       # Telegram Bot（含指令處理）
│   └── weather.h            # OpenWeatherMap 天氣 API
└── data/
    └── faces/               # SPIFFS 人臉資料庫目錄
```

## 🚀 PIR 近接喚醒

### 系統行為

```
休眠狀態 (STATE_SLEEP)
  • 門外 PIR 每 500ms 檢查一次
  • 門內 PIR 每 500ms 檢查一次（天氣播報）
  • OLED 關閉（省電）
  • 鏡頭暫停（不取 frame buffer）
  • WiFi + Telegram 維持（遠端開鎖仍可用）
  • 功耗約 70mA（vs 正常 200mA）
           ↓ 門外 PIR 偵測到人
喚醒過渡（~0.5秒）
  • OLED 亮起，顯示「Someone outside!」
  • 播放短暫喚醒提示音
  • 鏡頭重新啟動
           ↓
待機狀態 (STATE_IDLE)
  • 全功能運作（人臉 / 指紋 / 鍵盤 / OLED / Telegram）
  • 門外 PIR 持續監控
  • 門內 PIR 持續監控（天氣播報，有 5 分鐘冷卻）
  • 無人 15 秒 → 返回 SLEEP
```

### 可調整參數

在 `src/config.h` 中：

```cpp
#define SLEEP_TIMEOUT_SEC   15     // IDLE 狀態下，幾秒無人就進入休眠
#define PIR_COOLDOWN_SEC  300     // 門內 PIR 觸發天氣播報的冷卻時間（5分鐘）
```

## 🚀 快速開始

### 1. 環境設定

1. 安裝 [VS Code](https://code.visualstudio.com/)
2. 安裝 [PlatformIO IDE 擴充功能](https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)
3. 克隆或下載此專案

### 2. 設定配置

修改 `src/config.h` 中的以下參數：

```cpp
// WiFi 設定
#define WIFI_SSID          "你的WiFi名稱"
#define WIFI_PASS          "你的WiFi密碼"

// Telegram Bot 設定
#define BOT_TOKEN          "你的BotToken"    // 從 @BotFather 取得
#define CHAT_ID            "你的ChatID"      // 純數字

// 天氣 API 設定
#define OWM_API_KEY        "你的OWM_APIKey"  // 從 OpenWeatherMap 取得
#define OWM_CITY           "Taipei"

// 密碼設定（務必修改！）
#define DEFAULT_PASSWORD   "1234"            // 一般開鎖密碼
#define ADMIN_PASSWORD     "9999"            // 管理員密碼
```

### 3. 編譯與燒錄

1. 用 USB-C 連接 XIAO ESP32 S3 到電腦
2. 在 VS Code 中開啟專案
3. PlatformIO 會自動下載依賴庫（首次需數分鐘）
4. 點擊底部工具列的 **Upload** 按鈕（或按 `Ctrl+Alt+U`）
5. 上傳完成後，開啟 Serial Monitor（`Ctrl+Alt+S`，鮑率 115200）查看輸出

### 4. 上傳 SPIFFS 檔案系統

首次燒錄後，建議上傳 SPIFFS 以初始化人臉資料庫目錄：

1. 在 PlatformIO 選單中選擇 **Project Tasks**
2. 展開 **Platform** → **Upload Filesystem Image**

## 📱 Telegram Bot 指令

| 指令 | 功能 |
|------|------|
| `/unlock` | 遠端開鎖 5 秒 |
| `/status` | 查看系統狀態（IP、電量、人臉數等）|
| `/weather` | 查詢目前天氣 |
| `/battery` | 查看電池狀態 |
| `/alarm_off` | 遠端解除警報 |
| `/face_list` | 列出所有登錄人臉 |
| `/face_enroll [名稱]` | 登錄新人臉 |
| `/face_delete [名稱]` | 刪除指定人臉 |
| `/face_deleteall` | 清除所有人臉 |
| `/set_password [新密碼]` | 修改開鎖密碼 |
| `/help` | 顯示所有指令說明 |

**注意**：休眠期間仍可使用 `/unlock` 遠端開鎖，系統會自動喚醒。

## 🎯 使用方式

### 📱 系統啟動流程

1. **供電啟動**：系統上電後自動初始化所有體
2. **WiFi 連線**：連接到設定的 WiFi 網路
3. **進入休眠**：初始化完成後進入休眠模式等待喚醒
4. **PIR 偵測**：門外/門內 PIR 每 500ms 檢查一次

### 🚪 日常開鎖（5種方式）

#### 方式1：PIR 喚醒 + 人臉辨識
1. 走到門前（PIR 偵測到）
2. 系統自動喚醒，播放嗶聲，OLED 亮起
3. 站到鏡頭前，系統自動辨識（約 0.5~1 秒）
4. 辨識成功後門鎖打開，顯示歡迎訊息

#### 方式2：指紋開鎖
1. 按壓指紋感測器
2. 系統感應指紋並驗證
3. 指紋正確後門鎖打開

#### 方式3：密碼開鎖
1. 輸入 4~8 位數字密碼（預設：`1234`）
2. 按 `#` 確認
3. 密碼正確後門鎖打開
4. 按 `*` 可清除全部輸入
5. 按 `D` 可刪除最後一位

#### 方式4：管理員密碼開鎖
1. 先輸入管理員密碼（預設：`9999`）
2. 按 `A` 進入管理模式
3. 可進行人臉管理等操作

#### 方式5：Telegram 遠端開鎖
1. 使用手機傳送 `/unlock` 指令
2. 系統喚醒並開鎖
3. 收到解鎖成功通知

### ⌨️ 鍵盤按鍵說明

```
┌─────┬─────┬─────┬─────┐
│  1  │  2  │  3  │  A  │   A = 進入管理模式（需先輸入管理員密碼）
├─────┼─────┼─────┼─────┤
│  4  │  5  │  6  │  B  │
├─────┼─────┼─────┼─────┤
│  7  │  8  │  9  │  C  │
├─────┼─────┼─────┼─────┤
│  *  │  0  │  #  │  D  │   * = 清除輸入 / # = 確認密碼 / D = 刪除一位
└─────┴─────┴─────┴─────┘
```

### 🔐 密碼輸入流程

1. **喚醒系統**：走到門前或使用其他方式喚醒
2. **輸入密碼**：直接輸入數字鍵（不需按其他鍵）
3. **觀察顯示**：OLED 顯示輸入的位數 `****`
4. **確認**：按 `#` 確認密碼
5. **結果**：成功開鎖或顯示錯誤
6. **修正**：按 `D` 刪除最後一位，按 `*` 清除全部

**範例**：開鎖密碼為 `1234`
```
按 1 → 顯示 *
按 2 → 顯示 **
按 3 → 顯示 ***
按 4 → 顯示 ****
按 # → 門鎖打開！🎉
```

### 🛠️ 管理操作

#### 進入管理模式
1. 先輸入管理員密碼（預設：`9999`）
2. 按 `A` 鍵
3. 進入管理選單

#### 管理選單說明
| 按鍵 | 功能 | 說明 |
|------|------|------|
| `1` | 新增人臉 | 透過 Telegram 指令操作 |
| `2` | 刪除人臉 | 顯示操作說明 |
| `3` | 列出人臉 | 在 OLED 上顯示所有人臉 |
| `4` | 清除所有 | 刪除全部人臉（需二次確認）|
| `*` | 返回 | 回到待機狀態 |

### 👤 人臉登錄流程

1. 進入管理模式（`9999` + `A`）
2. 按 `1` 新增人臉
3. OLED 顯示操作說明
4. 使用 Telegram 傳送 `/face_enroll 姓名`
   ```
   範例：/face_enroll 王小明
   ```
5. Bot 回覆準備就緒
6. 站到鏡頭前，保持正面
7. 系統自動拍攝 5 張照片
8. 完成後 Bot 通知登錄成功

### 👆 指紋登錄流程

指紋模組需要透過實體按鍵操作，無法從 Telegram 遠端登錄：

1. 進入管理模式（先輸入管理密碼 `9999`，再按 `A`）
2. 按 `2` 選擇指紋登錄
3. 系統會提示「請按手指」
4. 在 AS608 指紋感測器上按壓手指
5. 聽到嗶聲後提起手指
6. 再次按壓同一手指（重複約 8-12 次）
7. 登錄成功後嗶嗶聲提示

**注意**：
- 登錄時請用同一手指重複按壓
- 每次按壓後需提起手指再重新按壓
- 登錄完成後可以測試解鎖功能

### 🔧 人臉刪除流程

1. 使用 Telegram 傳送 `/face_list` 查看所有人臉
2. 傳送 `/face_delete 姓名` 刪除指定人臉
   ```
   範例：/face_delete 王小明
   ```
3. Bot 確認刪除成功

### ⚡ 自動休眠

系統會在以下情況下自動進入休眠節省電力：
- **PIR 觸發**：待機狀態下，門外 PIR 無人偵測且 15 秒無操作
- **OLED 關閉**：休眠時 OLED 螢幕關閉
- **WiFi 維持**：WiFi 和 Telegram 在休眠期間仍正常運作
- **喚醒方式**：門外 PIR 偵測到人走近、Telegram 遠端指令

## ⚡ 電源管理

### 休眠模式省電

| 模式 | 功耗 |說明 |
|------|------|------|
| 正常運作 | ~200mA | 全功能開啟 |
| 休眠模式 | ~70mA | PIR 低頻檢測，OLED 關閉 |

### 冷卻機制

門內 PIR 觸發天氣播報後有 5 分鐘冷卻時間，避免人在門口走動時被瘋狂洗頻。

## ❓ 常見問題

### Q: 編譯時出現 "human_face_detect_msr01.hpp not found"
**A**: 執行 `PlatformIO: Clean` 後重新編譯，確保 platformio.ini 中的 include 路徑正確。

### Q: OLED 顯示花屏或白屏
**A**: 檢查 I2C 接線，確認 OLED 地址是 0x3C（部分模組為 0x3D）。

### Q: 人臉辨識準確率低
**A**: 
1. 增加登錄時的照片數量（修改 `FACE_ENROLL_SAMPLES`）
2. 登錄時拍攝不同角度（輕微左右轉頭）
3. 確保登錄和使用時光線條件相似
4. 考慮加裝 940nm 紅外補光燈

### Q: PIR 感測器誤觸發
**A**:
1. 調整安裝位置，避免正對窗戶或熱源
2. PIR 有約 30 秒預熱時間，安裝後等待穩定
3. 確認 VCC 為 3.3V（不要接 5V）

### Q: 門內天氣播報沒有觸發
**A**:
1. 檢查 PCF8574 #2 是否正確初始化
2. 確認冷卻時間（5 分鐘）已過
3. 檢查天氣 API 是否正常

## 📄 授權

本專案供學習和研究使用。

## 🙏 致謝

- [esp-face](https://github.com/espressif/esp-face) - 本地人臉辨識框架
- [Universal-Arduino-Telegram-Bot](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot) - Telegram Bot 庫
- [Adafruit_SSD1306](https://github.com/adafruit/Adafruit_SSD1306) - OLED 顯示庫

---

**版本**: V3.0（PIR 人體感測器版）  
**更新日期**: 2026/05/15  
**開發環境**: PlatformIO + Arduino Framework  
**目標硬體**: Seeed XIAO ESP32 S3 Sense