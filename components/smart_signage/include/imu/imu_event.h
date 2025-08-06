#pragma once
#include <etl/variant.h>
#include <etl/algorithm.h>
#include "imu/imu_const.h"

namespace esphome::smart_signage::imu {

// From user
struct CmdSetup {};
struct CmdStart {};
struct CmdStop {};
struct CmdTeardown {};
struct SetFallAngle {
    uint16_t deg;
    SetFallAngle(uint16_t d) : deg(clamp_max(d, kMaxFallAngleDeg)) {}
};
struct SetConfirmCnt {
    uint16_t cnt;
    SetConfirmCnt(uint16_t c) : cnt(clamp_min(c, kMinConfirmCount)) {}
};
struct SetSampleInt {
    uint32_t ms;
    SetSampleInt(uint32_t m) : ms(clamp_min(m, kMinSampleIntMs)) {}
};

// Internal
struct EvtTimerPoll {};
using Event = etl::variant<
    // Commands
    CmdSetup,
    CmdStart,
    CmdStop,
    CmdTeardown,

    SetFallAngle,
    SetConfirmCnt,
    SetSampleInt,

    EvtTimerPoll>;

} // namespace esphome::smart_signage::imu