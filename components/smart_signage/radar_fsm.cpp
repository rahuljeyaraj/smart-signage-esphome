#include "radar/radar_fsm.h"

namespace esphome::smart_signage::radar {

FSM::FSM(ctrl::Q &q) : ctrlQ_(q) {}

// Guard
bool FSM::SetupGuard(const CmdSetup &e) {
    LOGI(TAG, "onSetup: initializing hardware...");
    if (!stubHardwareInit()) {
        LOGE(TAG, "onSetup: hardware init failed");
        ctrlQ_.post(ctrl::EvtRadarError{});
        return false;
    }
    LOGI(TAG, "onSetup: success");
    ctrlQ_.post(ctrl::EvtRadarReady{});
    return true;
}

// Actions
void FSM::onCmdStart(const CmdStart &) { LOGI(TAG, "onStart: starting radar"); }

void FSM::onCmdStop(const CmdStop &) {
    LOGI(TAG, "onStop: stopping radar");
    // stubHardwareStop();
}

void FSM::onCmdTeardown(const CmdTeardown &) {
    LOGI(TAG, "onTeardown: tearing down");
    // stubHardwareTeardown();
}

void FSM::onEvtTimerPoll(const EvtTimerPoll &) {
    LOGI(TAG, "onPoll: reading data (maxDist=%u cm, interval=%u ms)", detDistCm_, sampleIntMs_);
    ctrlQ_.post(ctrl::EvtRadarData{true, 100});
    // stubHardwarePoll();
}

void FSM::onSetDist(const SetDistCm &c) {
    detDistCm_ = c.cm;
    LOGI(TAG, "onSetDist: set max distance to %u cm", detDistCm_);
}

void FSM::onSetSampleInt(const SetSampleInt &c) {
    sampleIntMs_ = c.ms;
    LOGI(TAG, "onSetSampleInt: set sample interval to %u ms", sampleIntMs_);
}

bool FSM::stubHardwareInit() {
    // Stub: return true for now.
    return false;
}

void FSM::onError() { LOGE(TAG, "Entered Error state!"); }

} // namespace esphome::smart_signage::radar
