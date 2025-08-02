#pragma once
#include <etl/variant.h>

namespace esphome::smart_signage::radar {

// ── commands sent *to* the radar
struct CmdSetup {};
struct CmdStart {};
struct CmdStop {};
struct CmdTeardown {};
struct EvtTimerPoll {};
struct SetDistCm {
    uint16_t cm;
};
struct SetSampleInt {
    uint32_t ms;
};

using Event = etl::variant<
    // Radar commands
    CmdSetup,
    CmdStart,
    CmdStop,
    CmdTeardown,

    EvtTimerPoll,

    SetDistCm,
    SetSampleInt>;

} // namespace esphome::smart_signage::radar