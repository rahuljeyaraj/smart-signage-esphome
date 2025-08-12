#pragma once
#include <cstdint>

namespace esphome::smart_signage::audio::hal {

// AO registers this; HAL calls it on natural EOF of a single item
using PlaybackDoneCb = void (*)(void *ctx);

class IAudioHAL {
  public:
    virtual ~IAudioHAL() = default;

    // lifecycle
    virtual bool init()   = 0;
    virtual bool deInit() = 0;

    // playback control
    virtual bool play(const char *source) = 0; // generic source
    virtual void stop()                   = 0;

    // volume
    virtual void setVolume(uint8_t pct) = 0;

    // AO needs to observe when a track naturally ends
    virtual void setPlaybackDoneCallback(PlaybackDoneCb cb, void *ctx) = 0;
};

} // namespace esphome::smart_signage::audio::hal
