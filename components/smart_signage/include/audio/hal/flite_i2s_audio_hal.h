#pragma once

#ifdef FLITE_I2S_AUDIO_HAL_H
// Disabled as consumes lot of Flash space,
// If required enable and addhttps://github.com/pschatzmann/arduino-flite.git to platformio.ini
#include <AudioTools.h>
#include <flite_arduino.h>
#include <iaudio_hal.h>
namespace esphome::smart_signage::audio::hal {
class FliteI2SAudioHal : public IAudioHAL {
  public:
    FliteI2SAudioHal(int bclkPin, int lrckPin, int dataPin) : i2s_(), flite_(i2s_) {
        i2sConfig_                 = i2s_.defaultConfig(audio_tools::TX_MODE);
        i2sConfig_.sample_rate     = 8000;
        i2sConfig_.channels        = 1;
        i2sConfig_.bits_per_sample = 16;
        i2sConfig_.pin_bck         = bclkPin;
        i2sConfig_.pin_ws          = lrckPin;
        i2sConfig_.pin_data        = dataPin;
    }

    ~FliteI2SAudioHal() override { deInit(); }

    bool init() override {
        if (initialized_) return true;

        if (!i2s_.begin(i2sConfig_)) return false;
        // Ins ESP32C3 Flash:154.8% , might work in ESP32S3
        // flite_.setVoice(register_cmu_us_slt(nullptr));

        initialized_ = true;

        return true;
    }

    bool deInit() override {
        if (!initialized_) return true;
        i2s_.end();
        initialized_ = false;
        return true;
    }

    // For TTS: 'fileName' is interpreted as the text to speak.
    bool play(const char *text) override {
        flite_.say(text);
        return true;
    }

    void stop() override {}

    void setVolume(uint8_t percent) override {}

    // Service TTS output; returns true if still speaking, false if done.
    bool service() override { return false; }

  private:
    I2SConfig i2sConfig_;
    I2SStream i2s_;
    Flite     flite_;
    bool      initialized_{false};
};
} // namespace esphome::smart_signage::audio::hal
#endif // FLITE_I2S_AUDIO_HAL_H
