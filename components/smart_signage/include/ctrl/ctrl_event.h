#pragma once
#include <etl/variant.h>
#include <freertos/FreeRTOS.h>

namespace esphome::smart_signage::ctrl {

struct CmdSetup {};
struct CmdStart {
    uint32_t runTimeMins;
};
struct CmdStop {};
struct CmdTeardown {};
struct EvtTimeout {};
struct EvtRadarError {};
struct EvtRadarReady {};
struct EvtRadarData {
    bool       detected       = false;
    uint16_t   distanceCm     = 0;
    TickType_t timestampTicks = 0;
};
struct EvtImuError {};
struct EvtImuReady {};
struct EvtImuFell {};
struct EvtImuRose {};
struct EvtLedError {};
struct EvtLedReady {};
struct EvtLedDone {};
struct EvtAudioError {};
struct EvtAudioReady {};
struct EvtAudioDone {};

// unified event variant
using Event = etl::variant<
    // commands
    CmdSetup,
    CmdStart,
    CmdStop,
    CmdTeardown,

    // timer
    EvtTimeout,

    // radar events
    EvtRadarError,
    EvtRadarReady,
    EvtRadarData,

    // imu events
    EvtImuError,
    EvtImuReady,
    EvtImuFell,
    EvtImuRose,

    // led events
    EvtLedError,
    EvtLedReady,
    EvtLedDone,

    // audio events
    EvtAudioError,
    EvtAudioReady,
    EvtAudioDone>;

} // namespace esphome::smart_signage::ctrl
