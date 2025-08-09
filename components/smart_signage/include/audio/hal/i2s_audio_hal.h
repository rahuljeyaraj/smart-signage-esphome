#pragma once
#include <Arduino.h>
#include <LittleFS.h> // ensure LittleFS.begin() at boot

#include <AudioToolsConfig.h>
#include <AudioTools/CoreAudio/AudioI2S/I2SStream.h> // or <AudioTools/CoreAudio/I2SStream.h>
#include <AudioTools/CoreAudio/VolumeStream.h>
#include <AudioTools/AudioCodecs/CodecMP3Helix.h>
#include <AudioTools/AudioCodecs/AudioEncoded.h>
#include <AudioTools/CoreAudio/StreamCopy.h>

#include "iaudio_hal.h"

namespace esphome::smart_signage::audio::hal {

class I2SAudioHal : public IAudioHAL {
  public:
    I2SAudioHal(int bclkPin, int lrckPin, int dataPin) : volume_(i2s_), dec_(&volume_, &mp3_) {
        i2s_cfg_          = i2s_.defaultConfig(audio_tools::TX_MODE);
        i2s_cfg_.pin_bck  = bclkPin;
        i2s_cfg_.pin_ws   = lrckPin;
        i2s_cfg_.pin_data = dataPin;
    }

    ~I2SAudioHal() override { deInit(); }

    bool init() override {
        if (initialized_) return true;
        if (!i2s_.begin(i2s_cfg_)) return false;

        dec_.addNotifyAudioChange(i2s_); // keep I2S in sync with decoder
        volume_.begin();                 // required before changing volume
        initialized_ = true;
        return true;
    }

    bool deInit() override {
        if (!initialized_) return true;
        stop();
        i2s_.end();
        initialized_ = false;
        return true;
    }

    bool play(const char *fileName) override {
        if (!initialized_) return false;
        stop();

        file_ = LittleFS.open(fileName, "r");
        if (!file_) return false;

        dec_.end();
        if (!dec_.begin()) return false;

        copier_.begin(dec_, file_); // File -> MP3 decoder -> Volume -> I2S
        active_ = true;
        return true;
    }

    void stop() override {
        active_ = false;
        copier_.end();
        dec_.end();
        if (file_) file_.close();
    }

    void setVolume(uint8_t percent) override {
        float v = (percent > 100 ? 100 : percent) / 100.0f;
        volume_.setVolume(v);
    }

    bool service() override {
        if (!active_) return false;
        size_t n = copier_.copy();
        if (n == 0 && !file_.available()) {
            stop();
            return false;
        }
        return true;
    }

  private:
    audio_tools::I2SConfig          i2s_cfg_;
    audio_tools::I2SStream          i2s_;
    audio_tools::MP3DecoderHelix    mp3_;
    audio_tools::VolumeStream       volume_{i2s_};
    audio_tools::EncodedAudioStream dec_{&volume_, &mp3_};
    audio_tools::StreamCopy         copier_;
    File                            file_;
    bool                            initialized_{false};
    bool                            active_{false};
};

} // namespace esphome::smart_signage::audio::hal
