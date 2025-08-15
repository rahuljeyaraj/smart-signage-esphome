#include "led/led_fsm.h"

namespace esphome::smart_signage::led {

FSM::FSM(ctrl::Q &ctrlQ, hal::ILedHal &hal, timer::ITimer &timer, timer::ITimer &fadeTimer)
    : ctrlQ_(ctrlQ), hal_(hal), timer_(timer), fadeTimer_(fadeTimer) {}

bool FSM::tryInitHal(const CmdSetup &) { return hal_.init(); }

bool FSM::breathUpGuard(const EvtTimerEnd &) const {
    return (!breath_.finite) || (breath_.cntLeft > 0);
}

void FSM::onSetupOk(const CmdSetup &) {
    ctrlQ_.post(ctrl::EvtLedReady{});
    SS_LOGI("setup ok");
}

void FSM::onSetupFail(const CmdSetup &) {
    ctrlQ_.post(ctrl::EvtLedError{});
    SS_LOGE("setup fail");
}

void FSM::onTeardown(const CmdTeardown &) {
    hal_.turnOff();
    breath_ = BreathCfg{};
    SS_LOGI("teardown");
}

void FSM::onError() {
    SS_LOGE("error");
    hal_.turnOff();
    breath_ = BreathCfg{};
}

void FSM::onCmdOn(const CmdOn &cmd) {
    breath_ = BreathCfg{};
    SS_LOGI("on");
    hal_.setBrightness(brightnessPct_);
}

void FSM::onEnterOff() {
    hal_.turnOff();
    SS_LOGI("off");
}

void FSM::setBreatheParams(const CmdBreathe &cmd) {
    breath_.toHighMs   = cmd.toHighMs;
    breath_.holdHighMs = cmd.holdHighMs;
    breath_.toLowMs    = cmd.toLowMs;
    breath_.holdLowMs  = cmd.holdLowMs;
    breath_.cntLeft    = cmd.cnt;
    breath_.finite     = cmd.cnt > 0;

    SS_LOGI("toHigh=%hu holdHigh=%hu toLow=%hu holdLow=%hu cnt=%hu%s",
        breath_.toHighMs,
        breath_.holdHighMs,
        breath_.toLowMs,
        breath_.holdLowMs,
        breath_.cntLeft,
        breath_.finite ? "" : " inf");
}

void FSM::onBreatheUpEntry() {
    hal_.fadeTo(brightnessPct_, breath_.toHighMs);
    const uint64_t us = static_cast<uint64_t>(breath_.toHighMs + kBufferMs) * 1000ULL;
    fadeTimer_.startOnce(us);
    SS_LOGD("enter up -> %hhu%% in %hu ms", brightnessPct_, breath_.toHighMs + kBufferMs);
}

void FSM::onHoldHighEntry() {
    const uint64_t us = static_cast<uint64_t>(breath_.holdHighMs) * 1000ULL;
    timer_.startOnce(us);
    SS_LOGD("enter holdHigh %hu ms", breath_.holdHighMs);
}

void FSM::onBreatheDownEntry() {
    hal_.fadeTo(0, breath_.toLowMs);
    const uint64_t us = static_cast<uint64_t>(breath_.toLowMs + kBufferMs) * 1000ULL;
    fadeTimer_.startOnce(us);
    SS_LOGD("enter down -> 0 in %hu ms", breath_.toLowMs + kBufferMs);
}

void FSM::onHoldLowEntry() {
    if (breath_.finite) --breath_.cntLeft;
    const uint64_t us = static_cast<uint64_t>(breath_.holdLowMs) * 1000ULL;
    timer_.startOnce(us);
    SS_LOGD("enter holdLow %hu ms", breath_.holdLowMs);
}

void FSM::finishBreathe(const EvtTimerEnd &) {
    ctrlQ_.post(ctrl::EvtLedDone{});
    SS_LOGI("breathe done");
    breath_ = BreathCfg{};
}

void FSM::onSetBrightness(const SetBrightness &e) { brightnessPct_ = e.pct; }

} // namespace esphome::smart_signage::led
