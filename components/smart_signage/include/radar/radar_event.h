#pragma once
#include <etl/variant.h>
#include "radar/radar_const.h"

namespace esphome::smart_signage::radar {

// From user
struct CmdSetup {};
struct CmdStart {};
struct CmdStop {};
struct CmdTeardown {};
struct SetDistCm {
    uint16_t cm;
};
struct SetSampleInt {
    uint32_t ms;
};

// Internal
struct EvtTimerPoll {};

using Event = etl::variant<
    // Radar commands
    CmdSetup,
    CmdStart,
    CmdStop,
    CmdTeardown,

    SetDistCm,
    SetSampleInt,

    EvtTimerPoll>;

} // namespace esphome::smart_signage::radar