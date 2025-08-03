#pragma once
#include <etl/variant.h>
#include <etl/algorithm.h>

namespace esphome::smart_signage::led {

constexpr uint16_t kMinBrightPct     = 0;
constexpr uint16_t kMaxBrightPct     = 100;
constexpr uint16_t kDefaultBrightPct = 100;

constexpr uint16_t kMinFadeMs      = 0;
constexpr uint16_t kMaxFadeMs      = 60000;
constexpr uint16_t kDefaultFadeIn  = 1000;
constexpr uint16_t kDefaultFadeOut = 1000;

constexpr uint16_t kMinCycles     = 0;
constexpr uint16_t kMaxCycles     = 1000;
constexpr uint16_t kDefaultCycles = 255;

// ── commands sent to the led q
struct CmdSetup {};
struct CmdTeardown {};
struct CmdOn {
    uint8_t brightPct = 100;
    CmdOn(uint16_t p = kDefaultBrightPct)
        : brightPct(etl::clamp(p, kMinBrightPct, kMaxBrightPct)) {}
};
struct CmdOff {};
struct CmdBreathe {
    uint8_t  brightPct;
    uint16_t fadeInMs;
    uint16_t fadeOutMs;
    uint16_t maxCycles;

    CmdBreathe(uint16_t p      = kDefaultBrightPct,
        uint16_t        in     = kDefaultFadeIn,
        uint16_t        out    = kDefaultFadeOut,
        uint16_t        cycles = kDefaultCycles)
        : brightPct(static_cast<uint8_t>(etl::clamp(p, kMinBrightPct, kMaxBrightPct))),
          fadeInMs(etl::clamp(in, kMinFadeMs, kMaxFadeMs)),
          fadeOutMs(etl::clamp(out, kMinFadeMs, kMaxFadeMs)),
          maxCycles(etl::clamp(cycles, kMinCycles, kMaxCycles)) {}
};
struct EvtFadeEnd {};
struct EvtTimerEnd {};

using Event = etl::variant<
    // Led commands
    CmdSetup,
    CmdStart,
    CmdStop,
    CmdTeardown,

    EvtFadeEnd,
    EvtTimerEnd>;

} // namespace esphome::smart_signage::led