#pragma once
#include <etl/variant.h>
#include <etl/algorithm.h>
#include "led/led_const.h"

namespace esphome::smart_signage::led {

// User events
struct CmdSetup {};
struct CmdTeardown {};
struct CmdOff {};
struct CmdOn {};
struct SetBrightness {
    uint8_t pct;
    SetBrightness(uint8_t p) : pct(etl::clamp<uint32_t>(p, kMinBrightPct, kMaxBrightPct)) {}
};

/**
 * Breath Waveform:
 *     highLevel   ____________
 *                /            \
 *               /              \                  (repeat)
 *              /                \               /
 *   lowLevel _/                  \_____________/
 *            ^   ^            ^   ^           ^
 *            |   |            |   |           |
 *            |   | holdHighMs |   | holdLowMs |
 *          toHighMs          toLowMs
 */
struct CmdBreathe {
    uint16_t toHighMs;   // ramp low -> high
    uint16_t holdHighMs; // dwell at high
    uint16_t toLowMs;    // ramp high -> low
    uint16_t holdLowMs;  // dwell at low (cycle ends after this)
    uint16_t cnt;        // number of full cycles; 0 => infinite

    CmdBreathe() : toHighMs(0), holdHighMs(0), toLowMs(0), holdLowMs(0), cnt(0) {}

    CmdBreathe(uint16_t tHigh, uint16_t hHigh, uint16_t tLow, uint16_t hLow, uint16_t n)
        : toHighMs(tHigh), holdHighMs(hHigh), toLowMs(tLow), holdLowMs(hLow), cnt(n) {}
};

// Internal events
struct EvtTimerEnd {};
struct EvtFadeEnd {};

using Event = etl::variant<CmdSetup, CmdTeardown, CmdOn, CmdOff, SetBrightness, CmdBreathe,
    EvtTimerEnd, EvtFadeEnd>;

} // namespace esphome::smart_signage::led
