#pragma once
#include <etl/variant.h>
#include <etl/algorithm.h>

namespace esphome::smart_signage::imu {

static constexpr uint16_t kMinFallAngleDeg     = 1;
static constexpr uint16_t kMaxFallAngleDeg     = 90;
static constexpr uint16_t kDefaultFallAngleDeg = 45;

static constexpr uint16_t kMinConfirmCount     = 2;
static constexpr uint16_t kMaxConfirmCount     = 50;
static constexpr uint16_t kDefaultConfirmCount = 5;

static constexpr uint32_t kMinSampleIntMs     = 10;
static constexpr uint32_t kMaxSampleIntMs     = 10000;
static constexpr uint32_t kDefaultSampleIntMs = 200;

// ── commands sent *to* the imu
struct CmdSetup {};
struct CmdStart {};
struct CmdStop {};
struct CmdTeardown {};
struct EvtTimerPoll {};
struct SetFallAngle {
    uint16_t deg;
    SetFallAngle(uint16_t d = kDefaultFallAngleDeg)
        : deg(etl::clamp(d, kMinFallAngleDeg, kMaxFallAngleDeg)) {}
};
struct SetConfirmCnt {
    uint16_t cnt;
    SetConfirmCnt(uint16_t c = kDefaultConfirmCount)
        : cnt(etl::clamp(c, kMinConfirmCount, kMaxConfirmCount)) {}
};
struct SetSampleInt {
    uint32_t ms;
    SetSampleInt(uint32_t m = kDefaultSampleIntMs)
        : ms(etl::clamp(m, kMinSampleIntMs, kMaxSampleIntMs)) {}
};

using Event = etl::variant<
    // Imu commands
    CmdSetup,
    CmdStart,
    CmdStop,
    CmdTeardown,

    EvtTimerPoll,

    SetFallAngle,
    SetConfirmCnt,
    SetSampleInt>;

} // namespace esphome::smart_signage::imu