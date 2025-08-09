#include "led/led_fsm.h"

namespace esphome::smart_signage::led {

FSM::FSM(ctrl::Q &ctrlQ, hal::ILedHal &hal, timer::ITimer &timer)
    : ctrlQ_(ctrlQ), hal_(hal), timer_(timer) {}

bool FSM::tryInitHal(const CmdSetup &) { return hal_.init(); }

bool FSM::breathUpGuard(const EvtTimerEnd &) const {
    return (!breath_.finite) || (breath_.cntLeft > 0);
}

void FSM::onSetupOk(const CmdSetup &) {
    ctrlQ_.post(ctrl::EvtLedReady{});
    LOGI("setup ok");
}

void FSM::onSetupFail(const CmdSetup &) {
    ctrlQ_.post(ctrl::EvtLedError{});
    LOGE("setup fail");
}

void FSM::onTeardown(const CmdTeardown &) {
    hal_.turnOff();
    breath_ = BreathCfg{};
    LOGI("teardown");
}

void FSM::onError() {
    LOGE("error");
    hal_.turnOff();
    breath_ = BreathCfg{};
}

void FSM::onCmdOn(const CmdOn &cmd) {
    breath_ = BreathCfg{};
    LOGI("on %lu%%", cmd.brightPct);
    hal_.setBrightness(static_cast<uint8_t>(cmd.brightPct));
}

void FSM::onEnterOff() {
    hal_.turnOff();
    LOGI("off");
}

void FSM::setBreatheParams(const CmdBreathe &cmd) {
    breath_.brightPct  = static_cast<uint8_t>(cmd.brightPct);
    breath_.toHighMs   = cmd.toHighMs;
    breath_.holdHighMs = cmd.holdHighMs;
    breath_.toLowMs    = cmd.toLowMs;
    breath_.holdLowMs  = cmd.holdLowMs;
    breath_.cntLeft    = cmd.cnt;
    breath_.finite     = cmd.cnt > 0;

    LOGI("breathe bright=%hhu%% toHigh=%hu holdHigh=%hu toLow=%hu holdLow=%hu cnt=%hu%s",
        breath_.brightPct,
        breath_.toHighMs,
        breath_.holdHighMs,
        breath_.toLowMs,
        breath_.holdLowMs,
        breath_.cntLeft,
        breath_.finite ? "" : " inf");
}

void FSM::onBreatheUpEntry() {
    hal_.fadeTo(breath_.brightPct, breath_.toHighMs);
    LOGD("enter up -> %hhu%% in %hu ms", breath_.brightPct, breath_.toHighMs);
}

void FSM::onHoldHighEntry() {
    const uint64_t us = static_cast<uint64_t>(breath_.holdHighMs) * 1000ULL;
    timer_.startOnce(us);
    LOGD("enter holdHigh %hu ms", breath_.holdHighMs);
}

void FSM::onBreatheDownEntry() {
    hal_.fadeTo(0, breath_.toLowMs);
    LOGD("enter down -> 0 in %hu ms", breath_.toLowMs);
}

void FSM::onHoldLowEntry() {
    if (breath_.finite) --breath_.cntLeft;
    const uint64_t us = static_cast<uint64_t>(breath_.holdLowMs) * 1000ULL;
    timer_.startOnce(us);
    LOGD("enter holdLow %hu ms", breath_.holdLowMs);
}

void FSM::finishBreathe(const EvtTimerEnd &) {
    ctrlQ_.post(ctrl::EvtLedDone{});
    LOGI("breathe done");
    breath_ = BreathCfg{};
}

} // namespace esphome::smart_signage::led
