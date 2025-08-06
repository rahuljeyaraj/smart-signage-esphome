#pragma once
#include <etl/variant.h>
#include "imu/imu_const.h"

namespace esphome::smart_signage::imu {

// From user
struct CmdSetup {};
struct CmdStart {};
struct CmdStop {};
struct CmdTeardown {};
struct SetFallAngle {
    uint16_t deg;
    SetFallAngle(uint16_t d) : deg(d > kMaxFallAngleDeg ? kMaxFallAngleDeg : d) {}
};
struct SetConfirmCnt {
    uint16_t cnt;
    SetConfirmCnt(uint16_t c) : cnt(c < kMinConfirmCount ? kMinConfirmCount : c) {}
};
struct SetSampleInt {
    uint32_t ms;
    SetSampleInt(uint32_t m) : ms(m < kMinSampleIntMs ? kMinSampleIntMs : m) {}
};

// Internal
struct EvtTimerPoll {};
using Event = etl::variant<
    // Commands
    CmdSetup, CmdStart, CmdStop, CmdTeardown,

    SetFallAngle, SetConfirmCnt, SetSampleInt,

    EvtTimerPoll>;

} // namespace esphome::smart_signage::imu