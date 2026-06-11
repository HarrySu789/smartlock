// src/audio.h
#pragma once

#include <driver/i2s.h>
#include "config.h"
#include <math.h>

// I2S 驅動設定
#define I2S_PORT         I2S_NUM_0
#define SAMPLE_RATE      22050
#define SAMPLE_BITS      I2S_BITS_PER_SAMPLE_16BIT
#define AUDIO_AMPLITUDE  8000    // 0~32767，越大越響（注意不要破音）

// 音效定義：{ 頻率Hz, 持續ms }，結尾用 {0, 0}
struct ToneNote { int freq; int duration; };

static const ToneNote SOUND_STARTUP[]   = {{880,80},{1175,80},{1760,120},{0,0}};
static const ToneNote SOUND_UNLOCK[]    = {{784,80},{988,80},{1319,150},{0,0}};
static const ToneNote SOUND_DENY[]      = {{350,200},{250,300},{0,0}};
static const ToneNote SOUND_BEEP[]      = {{1200,60},{0,0}};
static const ToneNote SOUND_ALARM[]     = {{2000,180},{0,120},{2000,180},{0,120},{2000,180},{0,0}};
static const ToneNote SOUND_ENROLL_OK[] = {{1047,80},{1319,80},{1568,120},{0,0}};

bool audioInitialized = false;

// ── 初始化 I2S ──────────────────────────────────
void initAudio() {
    i2s_config_t cfg = {
        .mode                 = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate          = SAMPLE_RATE,
        .bits_per_sample      = SAMPLE_BITS,
        .channel_format       = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags     = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count        = 4,
        .dma_buf_len          = 256,
        .use_apll             = false,
        .tx_desc_auto_clear   = true,
        .fixed_mclk           = 0
    };

    i2s_pin_config_t pins = {
        .bck_io_num   = I2S_BCLK_PIN,
        .ws_io_num    = I2S_LRCLK_PIN,
        .data_out_num = I2S_DATA_PIN,
        .data_in_num  = I2S_PIN_NO_CHANGE
    };

    esp_err_t err = i2s_driver_install(I2S_PORT, &cfg, 0, NULL);
    if (err != ESP_OK) {
        Serial.printf("❌ I2S 安裝失敗: %d\n", err);
        return;
    }
    i2s_set_pin(I2S_PORT, &pins);
    i2s_zero_dma_buffer(I2S_PORT);

    audioInitialized = true;
    Serial.println("✅ I2S 音頻初始化完成");
}

// ── 產生並輸出單個音符（正弦波）──────────────
void playNote(int freqHz, int durationMs) {
    if (!audioInitialized || freqHz <= 0 || durationMs <= 0) {
        if (durationMs > 0) delay(durationMs);
        return;
    }

    int totalSamples = (SAMPLE_RATE * durationMs) / 1000;
    const int BUF_SIZE = 512;
    int16_t buf[BUF_SIZE * 2];   // 立體聲，左右各一份

    int written = 0;
    while (written < totalSamples) {
        int chunk = min(BUF_SIZE, totalSamples - written);
        for (int i = 0; i < chunk; i++) {
            float t = (float)(written + i) / SAMPLE_RATE;
            int16_t sample = (int16_t)(AUDIO_AMPLITUDE *
                             sinf(2.0f * M_PI * freqHz * t));
            buf[i * 2]     = sample;  // 左聲道
            buf[i * 2 + 1] = sample;  // 右聲道
        }
        size_t bytesOut;
        i2s_write(I2S_PORT, buf, chunk * 4, &bytesOut, portMAX_DELAY);
        written += chunk;
    }
}

// ── 播放音效序列 ──────────────────────────────
void playSound(const ToneNote* notes) {
    for (int i = 0; notes[i].freq != 0 || notes[i].duration != 0; i++) {
        playNote(notes[i].freq, notes[i].duration);
    }
}

// ── 非同步播放（使用 FreeRTOS task，不阻塞主迴圈）──
struct AsyncSoundArg { const ToneNote* notes; };

void asyncSoundTask(void* arg) {
    auto* a = (AsyncSoundArg*)arg;
    playSound(a->notes);
    delete a;
    vTaskDelete(NULL);
}

void playSoundAsync(const ToneNote* notes) {
    auto* arg = new AsyncSoundArg{notes};
    xTaskCreate(asyncSoundTask, "snd", 4096, arg, 1, NULL);  // 增加堆疊大小到 4KB
}

// ── 天氣 TTS 語音播報 ──────────────────────────────
// 將文字轉換為簡單的音節發音（英文/中文數字）
void playWeatherTTS(const String& text) {
    if (!audioInitialized) return;
    
    Serial.printf("🔊 語音播報: %s\n", text.c_str());
    
    // 播放提示音
    playNote(1200, 150);
    delay(100);
    
    // 根據天氣訊息播放不同音調
    if (text.indexOf("雨") >= 0 || text.indexOf("rain") >= 0) {
        // 下雨：播放較低的音調
        playNote(440, 300);  // A4
        delay(100);
        playNote(392, 300);  // G4
        delay(100);
        playNote(349, 400);  // F4
    } else {
        // 好天氣：播放較高的音調
        playNote(523, 200);  // C5
        delay(80);
        playNote(659, 200);  // E5
        delay(80);
        playNote(784, 300);  // G5
    }
    
    delay(200);
    // 結束提示音
    playNote(1200, 100);
}

// ── 天氣音效定義 ──────────────────────────────
static const ToneNote SOUND_WEATHER_SUNNY[] = {{523,150},{659,150},{784,200},{0,0}};
static const ToneNote SOUND_WEATHER_RAIN[] = {{392,150},{349,150},{293,200},{0,0}};
static const ToneNote SOUND_WEATHER_CLOUDY[] = {{440,150},{494,150},{523,200},{0,0}};

// ══════════════════════════════════════════════════════════════════
//  自製 WAV 播放器 - 直接讀取 WAV 並通過 I2S 播放
//  不依賴 ESP8266Audio 函式庫，避免衝突和崩潰
// ══════════════════════════════════════════════════════════
#include <SPIFFS.h>

// WAV 播放狀態
static bool wavReady = false;

// WAV 文件結構
struct WavHeader {
    char     riff[4];       // "RIFF"
    uint32_t fileSize;      // 檔案大小 - 8
    char     wave[4];       // "WAVE"
    char     fmt[4];        // "fmt "
    uint32_t fmtSize;      // fmt chunk 大小 (通常 16)
    uint16_t audioFormat;   // 音頻格式 (1=PCM)
    uint16_t numChannels;  // 聲道數
    uint32_t sampleRate;  // 取樣率
    uint32_t byteRate;   // 位元組速率
    uint16_t blockAlign;    // 區塊對齊
    uint16_t bitsPerSample;// 位元/樣本
    char     data[4];       // "data"
    uint32_t dataSize;     // 資料大小
};

// 檢查並顯示 WAV 資訊
bool parseWavHeader(WavHeader* hdr, File& file) {
    // 讀取 RIFF WAVE header
    if (file.readBytes(hdr->riff, 4) != 4) return false;
    if (memcmp(hdr->riff, "RIFF", 4) != 0) return false;
    
    file.readBytes((char*)&hdr->fileSize, 4);
    if (file.readBytes(hdr->wave, 4) != 4) return false;
    if (memcmp(hdr->wave, "WAVE", 4) != 0) return false;
    
    // 讀取 fmt chunk
    if (file.readBytes(hdr->fmt, 4) != 4) return false;
    if (file.readBytes((char*)&hdr->fmtSize, 4) != 4) return false;
    if (file.readBytes((char*)&hdr->audioFormat, 2) != 2) return false;
    if (file.readBytes((char*)&hdr->numChannels, 2) != 2) return false;
    if (file.readBytes((char*)&hdr->sampleRate, 4) != 4) return false;
    if (file.readBytes((char*)&hdr->byteRate, 4) != 4) return false;
    if (file.readBytes((char*)&hdr->blockAlign, 2) != 2) return false;
    if (file.readBytes((char*)&hdr->bitsPerSample, 2) != 2) return false;
    
    // 跳過可能的其他 chunks直到 data
    while (file.readBytes(hdr->data, 4) == 4) {
        if (memcmp(hdr->data, "data", 4) == 0) {
            // 找到 data chunk
            file.readBytes((char*)&hdr->dataSize, 4);
            break;
        }
        // 否則，跳過這個 chunk
        uint32_t chunkSize;
        file.readBytes((char*)&chunkSize, 4);
        file.seek(file.position() + chunkSize);
    }
    
    Serial.printf("🔊 WAV: ch=%d, rate=%lu, bits=%d, size=%lu\n",
        hdr->numChannels, hdr->sampleRate, hdr->bitsPerSample, hdr->dataSize);
    
    return true;
}

// 簡化的 Fallback 音效函式
void playFallbackSound(const char* filepath) {
    Serial.println("🔄 [Audio] 使用 Fallback 合成音效...");
    // 檢查檔案名稱中的關鍵字
    bool isRain = (strstr(filepath, "rain") != NULL);
    bool isCloudy = (strstr(filepath, "cloudy") != NULL);
    
    if (isRain) playSound(SOUND_WEATHER_RAIN);
    else if (isCloudy) playSound(SOUND_WEATHER_CLOUDY);
    else playSound(SOUND_WEATHER_SUNNY);
}

void playWavSync(String filepath) {
    if (!audioInitialized) {
        Serial.println("⚠️ [Audio] 需要先 initAudio");
        playFallbackSound(filepath.c_str());
        return;
    }
    
    File file = SPIFFS.open(filepath.c_str());
    if (!file) {
        Serial.printf("❌ [Audio] 無法打開檔案: %s\n", filepath.c_str());
        // 檔案打開失敗，嘗試播放Fallback音效
        playFallbackSound(filepath.c_str());
        return;
    }
    
    WavHeader hdr;
    memset(&hdr, 0, sizeof(hdr));
    
    if (file.read((uint8_t*)hdr.riff, 4) != 4 || memcmp(hdr.riff, "RIFF", 4) != 0) { file.close(); return; }
    file.read((uint8_t*)&hdr.fileSize, 4);
    if (file.read((uint8_t*)hdr.wave, 4) != 4 || memcmp(hdr.wave, "WAVE", 4) != 0) { file.close(); return; }
    if (file.read((uint8_t*)hdr.fmt, 4) != 4) { file.close(); return; }
    
    file.read((uint8_t*)&hdr.fmtSize, 4);
    file.read((uint8_t*)&hdr.audioFormat, 2);
    file.read((uint8_t*)&hdr.numChannels, 2);
    file.read((uint8_t*)&hdr.sampleRate, 4);
    file.read((uint8_t*)&hdr.byteRate, 4);
    file.read((uint8_t*)&hdr.blockAlign, 2);
    file.read((uint8_t*)&hdr.bitsPerSample, 2);
    
    int extraFmtBytes = hdr.fmtSize - 16;
    if (extraFmtBytes > 0) file.seek(file.position() + extraFmtBytes);
    
    char chunkId[4];
    uint32_t chunkSize;
    while (file.available()) {
        if (file.read((uint8_t*)chunkId, 4) != 4) break;
        if (file.read((uint8_t*)&chunkSize, 4) != 4) break;
        if (memcmp(chunkId, "data", 4) == 0) {
            hdr.dataSize = chunkSize;
            break;
        }
        uint32_t skip = chunkSize;
        if (skip % 2 != 0) skip++;
        file.seek(file.position() + skip);
    }

    Serial.printf("🔊 WAV: ch=%d, rate=%lu, bits=%d, size=%lu\n",
        hdr.numChannels, (unsigned long)hdr.sampleRate, hdr.bitsPerSample, (unsigned long)hdr.dataSize);

    // =========================================================
    // 🚀 魔法除錯開關區 (請每次只把一個改成 true 來測試)
    // =========================================================
    bool FIX_BYTE_SWAP = false;     // 開關 1：測試高低位元反轉
    bool FIX_1_BYTE_OFFSET = false; // 開關 2：測試跳過 1 Byte 錯位
    bool FIX_UNSIGNED = false;     // 開關 3：測試修正無符號格式 (通常是 WAV PCM 格式需要)
    // =========================================================

    if (FIX_1_BYTE_OFFSET) file.read(); // 強制位移 1 Byte

    uint32_t bytesPerSample = (hdr.bitsPerSample / 8) * hdr.numChannels;
    uint32_t totalFrames = hdr.dataSize / bytesPerSample;
    const size_t BUF_FRAMES = 256; 
    int16_t rawBuf[BUF_FRAMES * 2];
    int16_t stereoBuf[BUF_FRAMES * 2];
    uint32_t played = 0;
    
    // 播放前徹底清空硬體緩衝，避免殘留雜訊
    i2s_zero_dma_buffer(I2S_PORT);
    
    while (played < totalFrames && file.available()) {
        uint32_t framesToRead = min((uint32_t)BUF_FRAMES, totalFrames - played);
        size_t bytesRead = file.read((uint8_t*)rawBuf, framesToRead * bytesPerSample);
        if (bytesRead == 0) break;
        
        size_t framesRead = (bytesRead - (bytesRead % bytesPerSample)) / bytesPerSample;
        
        for (uint32_t i = 0; i < framesRead; i++) {
            int16_t sample = rawBuf[i];

            // 應用魔法開關
            if (FIX_BYTE_SWAP) {
                sample = ((sample & 0xFF) << 8) | ((sample >> 8) & 0xFF);
            }
            if (FIX_UNSIGNED) {
                sample = sample - 32768; 
            }

            // 輸出前 5 個樣本，讓我們肉眼看穿波形！
            if (played == 0 && i < 5) {
                Serial.printf("📊 波形樣本 [%d]: %d\n", i, sample);
            }

            // 降音量避免破音
            sample = sample / 4; 

            if (hdr.numChannels == 1) {
                stereoBuf[i * 2]     = sample;
                stereoBuf[i * 2 + 1] = sample;
            } else {
                stereoBuf[i * 2]     = sample;
                stereoBuf[i * 2 + 1] = rawBuf[i * 2 + 1] / 4; // 這裡暫不考慮立體聲的進階除錯
            }
        }
        
        size_t bytesWritten;
        i2s_write(I2S_PORT, stereoBuf, framesRead * 4, &bytesWritten, portMAX_DELAY);
        played += framesRead;
    }
    
    // 延遲一下讓 DMA 把最後一波聲音播完再關閉
    delay(100); 
    file.close();
    Serial.println("✅ [Audio] 播放完成");
}