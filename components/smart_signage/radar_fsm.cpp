#include "radar/radar_fsm.h"

namespace esphome::smart_signage::radar {

FSM::FSM(ctrl::Q &q) : ctrlQ_(q) {}

// Guard
bool FSM::isReadyGuard(const CmdSetup &e) {
    LOGI("onSetup: initializing hardware...");
    if (!stubHardwareInit()) {
        LOGE("onSetup: hardware init failed");
        ctrlQ_.post(ctrl::EvtRadarError{});
        return false;
    }
    LOGI("onSetup: success");
    ctrlQ_.post(ctrl::EvtRadarReady{});
    return true;
}

// Actions
void FSM::onCmdStart(const CmdStart &) { LOGI("onStart: starting radar"); }

void FSM::onCmdStop(const CmdStop &) {
    LOGI("onStop: stopping radar");
    // stubHardwareStop();
}

void FSM::onCmdTeardown(const CmdTeardown &) {
    LOGI("onTeardown: tearing down");
    // stubHardwareTeardown();
}

void FSM::onEvtTimerPoll(const EvtTimerPoll &) {
    LOGI("onPoll: reading data (maxDist=%u cm, interval=%u ms)", detDistCm_, sampleIntMs_);
    ctrlQ_.post(ctrl::EvtRadarData{true, 100});
    // stubHardwarePoll();
}

void FSM::onSetDist(const SetRangeCm &c) {
    detDistCm_ = c.cm;
    LOGI("onSetDist: set max distance to %u cm", detDistCm_);
}

void FSM::onSetSampleInt(const SetSampleInt &c) {
    sampleIntMs_ = c.ms;
    LOGI("onSetSampleInt: set sample interval to %u ms", sampleIntMs_);
}

bool FSM::stubHardwareInit() {
    // Stub: return true for now.
    return true;
}

void FSM::onError() { LOGE("Entered Error state!"); }

} // namespace esphome::smart_signage::radar
