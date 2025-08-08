#include "radar/radar_fsm.h"
#include "log.h"

namespace esphome::smart_signage::radar {

FSM::FSM(ctrl::Q &q, hal::IRadarHal &hal, timer::ITimer &t, SimpleKalmanFilter &f)
    : ctrlQ_(q), hal_(hal), timer_(t), filter_(f) {}

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

void FSM::onCmdStart(const CmdStart &) {
    LOGD("onStart: radar polling at %u ms", sampleIntMs_);
    // Starts the timer
    timer_.startPeriodic(uint64_t(sampleIntMs_) * 1000ULL);
}

void FSM::onCmdStop(const CmdStop &) { timer_.stop(); }

void FSM::onCmdTeardown(const CmdTeardown &) { timer_.stop(); }

void FSM::onEvtTimerPoll(const EvtTimerPoll &) {
    if (!hal_.hasNewData()) { LOGW("No new radar data"); }

    uint16_t raw    = hal_.getDistance();
    bool     pres   = hal_.presenceDetected();
    auto     filt   = static_cast<uint16_t>(filter_.updateEstimate(raw) + 0.5f);
    bool     detect = pres && (filt <= detDistCm_);

    LOGD("raw=%u filtered=%u detected=%s", raw, filt, detect ? "YES" : "NO");
    ctrlQ_.post(ctrl::EvtRadarData{detect, filt, xTaskGetTickCount()});
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
