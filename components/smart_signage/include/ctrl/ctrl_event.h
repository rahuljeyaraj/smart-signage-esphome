#pragma once
#include <etl/variant.h>
#include <freertos/FreeRTOS.h>
#include "ctrl/ctrl_const.h"

namespace esphome::smart_signage::ctrl {

// From upper layer
struct CmdSetup {};
struct CmdStart {
    uint32_t sessionMins = kDefaultSessionMins;
};
struct CmdStop {};
struct CmdTeardown {};

// UI -> Controller commands (no dynamic allocations)
struct CmdUiSelectProfile {
    uint32_t idx;
};
struct CmdUiSetSessionMins {
    uint32_t mins;
};
struct CmdUiSetRangeCm {
    uint32_t cm;
};
struct CmdUiSetAudioVol {
    uint8_t pct;
};
struct CmdUiSetLedBright {
    uint8_t pct;
};
struct CmdUiPressStart {};

struct EvtSetupTimeout {};
struct EvtSessionEnd {};

// TODO: move these to respective intf headers
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
    // core commands
    CmdSetup, CmdStart, CmdStop, CmdTeardown,
    // UI commands
    CmdUiSelectProfile, CmdUiSetSessionMins, CmdUiSetRangeCm, CmdUiSetAudioVol, CmdUiSetLedBright,
    CmdUiPressStart,
    // timer
    EvtSetupTimeout, EvtSessionEnd,
    // radar events
    EvtRadarError, EvtRadarReady, EvtRadarData,
    // imu events
    EvtImuError, EvtImuReady, EvtImuFell, EvtImuRose,
    // led events
    EvtLedError, EvtLedReady, EvtLedDone,
    // audio events
    EvtAudioError, EvtAudioReady, EvtAudioDone>;

} // namespace esphome::smart_signage::ctrl
