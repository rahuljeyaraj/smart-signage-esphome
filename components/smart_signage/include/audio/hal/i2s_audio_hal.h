#pragma once
#include <Arduino.h>
#include <LittleFS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <AudioToolsConfig.h>
#include <AudioTools/CoreAudio/AudioI2S/I2SStream.h>
#include <AudioTools/CoreAudio/VolumeStream.h>
#include <AudioTools/AudioCodecs/CodecMP3Helix.h>
#include <AudioTools/AudioCodecs/AudioEncoded.h>
#include <AudioTools/CoreAudio/StreamCopy.h>

#include "log.h"
#include "audio/hal/iaudio_hal.h"

namespace esphome::smart_signage::audio::hal {

class I2SAudioHal : public IAudioHAL {
  public:
    I2SAudioHal(int bclkPin, int lrckPin, int dataPin) : volume_(i2s_), dec_(&volume_, &mp3_) {
        cfg_          = i2s_.defaultConfig(audio_tools::TX_MODE);
        cfg_.pin_bck  = bclkPin;
        cfg_.pin_ws   = lrckPin;
        cfg_.pin_data = dataPin;
        SS_LOGI("ctor BCLK=%d LRCK=%d DATA=%d", bclkPin, lrckPin, dataPin);
    }

    ~I2SAudioHal() override { deInit(); }

    bool init() override {
        if (initialized_) return true;
        if (!i2s_.begin(cfg_)) {
            SS_LOGE("i2s.begin failed");
            return false;
        }
        dec_.addNotifyAudioChange(i2s_);
        volume_.begin();

        // worker task (starts suspended; resume on play)
        BaseType_t ok = xTaskCreate(
            &I2SAudioHal::workerTrampoline, "AudioCopy", 8192, this, tskIDLE_PRIORITY + 2, &task_);
        if (ok != pdPASS) {
            task_ = nullptr;
            SS_LOGE("task create failed");
            return false;
        }
        vTaskSuspend(task_);

        initialized_ = true;
        SS_LOGI("init OK");
        return true;
    }

    bool deInit() override {
        stop(); // ensure idle
        if (task_) {
            vTaskDelete(task_);
            task_ = nullptr;
        }
        if (initialized_) {
            i2s_.end();
            initialized_ = false;
        }
        SS_LOGI("deinit OK");
        return true;
    }

    void setPlaybackDoneCallback(PlaybackDoneCb cb, void *ctx) override {
        cb_     = cb;
        cb_ctx_ = ctx;
    }

    bool play(const char *source) override {
        if (!initialized_ || !task_) {
            SS_LOGE("play not ready");
            return false;
        }
        if (!source || *source == '\0') {
            SS_LOGE("play bad source");
            return false;
        }

        // today: LittleFS; tomorrow: branch on prefix for TTS etc.
        if (!LittleFS.exists(source)) {
            SS_LOGE("not found: %s", source);
            return false;
        }

        // reset any prior pipeline
        stop_nocb_();

        file_ = LittleFS.open(source, "r");
        if (!file_) {
            SS_LOGE("open failed: %s", source);
            return false;
        }

        if (!dec_.begin()) {
            SS_LOGE("decoder begin failed");
            file_.close();
            return false;
        }

        copier_.begin(dec_, file_);
        userStopped_ = false;
        active_      = true;
        SS_LOGI("play %s", source);

        vTaskResume(task_);
        return true;
    }

    void stop() override {
        userStopped_ = true;
        stop_nocb_(); // no callback when user stops
    }

    void setVolume(uint8_t pct) override {
        if (pct > 100) pct = 100;
        const float v = static_cast<float>(pct) / 100.0f;
        volume_.setVolume(v);
        SS_LOGI("volume %hhu%%", pct);
    }

  private:
    static constexpr char TAG[] = "I2SAudioHal";

    static void workerTrampoline(void *arg) { static_cast<I2SAudioHal *>(arg)->worker(); }

    void worker() {
        for (;;) {
            if (!active_) {
                vTaskSuspend(nullptr);
                continue;
            }

            size_t moved = copier_.copy();

            if (moved == 0 && !file_.available()) {
                // natural EOF
                stop_nocb_();
                if (!userStopped_ && cb_) cb_(cb_ctx_);
                // next loop will suspend on !active_
            } else if (moved == 0) {
                vTaskDelay(pdMS_TO_TICKS(2)); // gentle yield on backpressure
            }
        }
    }

    void stop_nocb_() {
        active_ = false;
        copier_.end();
        dec_.end();
        if (file_) { file_.close(); }
        SS_LOGI("stop");
    }

  private:
    audio_tools::I2SConfig          cfg_{};
    audio_tools::I2SStream          i2s_{};
    audio_tools::MP3DecoderHelix    mp3_{};
    audio_tools::VolumeStream       volume_{i2s_};
    audio_tools::EncodedAudioStream dec_{&volume_, &mp3_};
    audio_tools::StreamCopy         copier_{};
    File                            file_{};

    TaskHandle_t   task_{nullptr};
    PlaybackDoneCb cb_{nullptr};
    void          *cb_ctx_{nullptr};

    bool initialized_{false};
    bool active_{false};
    bool userStopped_{false};
};

} // namespace esphome::smart_signage::audio::hal
