#include "radar/radar_fsm.h"
#include "log.h"

namespace esphome::smart_signage::radar {

FSM::FSM(ctrl::Q &q, hal::IRadarHal &hal, SimpleKalmanFilter &filter)
    : ctrlQ_(q), hal_(hal), filter_(filter) {}

bool FSM::isReadyGuard(const CmdSetup &e) {

    LOGI("onSetup: Initializing hardware...");
    if (!hal_.init()) {
        LOGE("onSetup: HAL init failed");
        ctrlQ_.post(ctrl::EvtRadarError{});
        return false;
    }
    LOGI("onSetup: success");
    ctrlQ_.post(ctrl::EvtRadarReady{});
    return true;
}

void FSM::onCmdStart(const CmdStart &) { LOGI("onStart: radar polling at %u ms", sampleIntMs_); }

void FSM::onCmdStop(const CmdStop &) {
    LOGI("onStop: stopping radar");
    // esphome teardown will stop EvtTimerPoll events
}

void FSM::onCmdTeardown(const CmdTeardown &) { LOGI("onTeardown: tearing down radar"); }

// **Core polling action**: replace stub with HAL + filter + threshold
void FSM::onEvtTimerPoll(const EvtTimerPoll &) {
    LOGI("onPoll: maxDist=%u cm, interval=%u ms", detDistCm_, sampleIntMs_);
    if (!hal_.hasNewData()) { LOGW("No new radar data"); }

    uint16_t raw    = hal_.getDistance();
    bool     pres   = hal_.presenceDetected();
    auto     filt   = static_cast<uint16_t>(filter_.updateEstimate(raw) + 0.5f);
    bool     detect = pres && (filt <= detDistCm_);

    LOGI("raw=%u filtered=%u detected=%s", raw, filt, detect ? "YES" : "NO");
    ctrlQ_.post(ctrl::EvtRadarData{detect, filt});
}

void FSM::onSetDist(const SetRangeCm &c) {
    detDistCm_ = c.cm;
    LOGI("onSetDist: max distance = %u cm", detDistCm_);
}

void FSM::onSetSampleInt(const SetSampleInt &c) {
    sampleIntMs_ = c.ms;
    LOGI("onSetSampleInt: interval = %u ms", sampleIntMs_);
}

void FSM::onError() { LOGE("Entered Error state!"); }

} // namespace esphome::smart_signage::radar
