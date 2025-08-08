#pragma once
#include <cstdint>

namespace esphome::smart_signage::timer {

class ITimer {
  public:
    using Callback = void (*)(void *);

    virtual ~ITimer() = default;

    /**
     * Configure the timer.
     * @param name short name for debugging
     * @param cb   function to call on expiry
     * @param arg  passed back to cb
     * @return      true if creation succeeded
     */
    virtual bool create(const char *name, Callback cb, void *arg) = 0;

    /** Start a one-shot timeout after timeout_us microseconds. */
    virtual void startOnce(uint64_t timeout_us) = 0;

    /** Start a periodic timer firing every period_us microseconds. */
    virtual void startPeriodic(uint64_t period_us) = 0;

    /** Stop the timer (if running). */
    virtual void stop() = 0;
};

} // namespace esphome::smart_signage::timer
