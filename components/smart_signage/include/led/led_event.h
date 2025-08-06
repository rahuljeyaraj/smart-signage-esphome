#pragma once
#include <etl/variant.h>
#include <etl/algorithm.h> // etl::clamp
#include "led/led_const.h"

namespace esphome::smart_signage::led {

// From User
struct CmdSetup {};
struct CmdTeardown {};
struct CmdOff {};
struct CmdOn {
    uint32_t brightPct;
    explicit CmdOn(uint32_t p = kDefaultBrightPct)
        : brightPct(etl::clamp(p, kMinBrightPct, kMaxBrightPct)) {}
};
struct CmdBreathe {
    uint32_t brightPct;
    uint16_t fadeInMs;
    uint16_t fadeOutMs;
    uint16_t maxCycles;
    CmdBreathe(uint32_t p, uint16_t in, uint16_t out, uint16_t cycles)
        : brightPct(etl::clamp(p, kMinBrightPct, kMaxBrightPct)) {}
};

// Internal
struct EvtFadeEnd {};

using Event = etl::variant<CmdSetup, CmdTeardown, CmdOn, CmdOff, CmdBreathe, EvtFadeEnd>;

} // namespace esphome::smart_signage::led
