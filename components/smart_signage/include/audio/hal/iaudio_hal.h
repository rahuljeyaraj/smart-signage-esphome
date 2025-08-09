#pragma once
#include <cstdint>

namespace esphome::smart_signage::audio::hal {
class IAudioHAL {
  public:
    virtual ~IAudioHAL()                    = default;
    virtual bool init()                     = 0;
    virtual bool play(const char *fileName) = 0;
    virtual void stop()                     = 0;
    virtual bool deInit()                   = 0;
    virtual void setVolume(uint8_t pct)     = 0;
    virtual bool service()                  = 0;
};
} // namespace esphome::smart_signage::audio::hal