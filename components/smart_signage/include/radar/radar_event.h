#pragma once
#include <etl/variant.h>
#include "radar/radar_const.h"

namespace esphome::smart_signage::radar {

// From user
struct CmdSetup {};
struct CmdStart {};
struct CmdStop {};
struct CmdTeardown {};
struct SetRangeCm {
    uint32_t cm;
};
struct SetSampleInt {
    uint32_t ms;
    SetSampleInt(uint32_t m) : ms(m < kMinSampleIntMs ? kMinSampleIntMs : m) {}
};

// Internal
struct EvtTimerPoll {};

using Event = etl::variant<
    // Radar commands
    CmdSetup, CmdStart, CmdStop, CmdTeardown,

    SetRangeCm, SetSampleInt,

    EvtTimerPoll>;

} // namespace esphome::smart_signage::radar