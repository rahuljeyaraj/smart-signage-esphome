#pragma once
#include <cstdint>

namespace esphome::smart_signage::led::hal {

class ILedHal {
  public:
    /// Callback prototype fired when a hardware fade finishes.
    using FadeEndCb = void (*)(void *cbCtx);

    virtual ~ILedHal() = default;

    /// Register a fade-end callback
    virtual void setFadeEndCallback(FadeEndCb cb, void *cbCtx) = 0;

    /// Initialise the peripheral
    virtual bool init() = 0;

    /// Immediately set brightness [0–100%].
    virtual bool setBrightness(uint8_t percent) = 0;

    /// Full off (stops PWM, drives pin low).
    virtual bool turnOff() = 0;

    /// Fade to `targetPercent` over `durationMs` milliseconds.
    virtual bool fadeTo(uint8_t targetPercent, uint32_t durationMs) = 0;

    /// Convenience alias for full on.
    virtual bool turnOn() { return setBrightness(100); }
};

} // namespace esphome::smart_signage::led::hal
