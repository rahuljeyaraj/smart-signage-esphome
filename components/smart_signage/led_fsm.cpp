#include "led/led_fsm.h"

namespace esphome::smart_signage::led {

/* ctor */
FSM::FSM(ctrl::Q &ctrlQ, hal::ILedHal &hal, timer::ITimer &timer)
    : ctrlQ_(ctrlQ), hal_(hal), timer_(timer) {}

/* guards */
bool FSM::hwInitGuard(const CmdSetup &) {
    LOGI("LED setup: init HAL");
    if (!hal_.init()) {
        LOGE("LED setup: HAL init failed");
        ctrlQ_.post(ctrl::EvtLedError{}); // This is fail action, move it to a action fn
        return false;
    }
    ctrlQ_.post(ctrl::EvtLedReady{}); // This is success action, move it to a action fn
    LOGI("LED setup: success");
    return true;
}

bool FSM::breatheUpGuard(const EvtFadeEnd &) {
    if (!breathCfg_.finite) return true;
    return --breathCfg_.cntLeft > 0;
}

/* top-level actions */
void FSM::onTeardown(const CmdTeardown &) {
    hal_.turnOff();
    LOGI("LED teardown -> Idle");
}

void FSM::onError() { LOGE("LED Error state"); }

void FSM::onCmdOn(const CmdOn &cmd) {
    hal_.setBrightness(cmd.brightPct);
    LOGI("LED On %u%%", (unsigned) cmd.brightPct);
}

void FSM::onEnterOff() {
    hal_.turnOff();
    LOGI("LED off");
}

void FSM::setBreathParams(const CmdBreathe &cmd) {
    breathCfg_.brightPct = cmd.brightPct;
    breathCfg_.fadeInMs  = cmd.fadeInMs;
    breathCfg_.fadeOutMs = cmd.fadeOutMs;
    breathCfg_.cntLeft   = cmd.cnt;
    breathCfg_.finite    = cmd.cnt > 0;
    // LOGI("Breath: %u%%, in: %u ms, out: %u ms, cycles: ");
}

void FSM::onBreatheUpEntry() {
    LOGI("onBreatheUpEntry");
    hal_.fadeTo(breathCfg_.brightPct, breathCfg_.fadeInMs);
}

void FSM::onBreatheDownEntry() {
    LOGI("onBreatheDownEntry");
    hal_.fadeTo(0, breathCfg_.fadeOutMs);
}

void FSM::onBreatheExit() {
    ctrlQ_.post(ctrl::EvtLedDone{});
    LOGD("Breathe completed");
}

} // namespace esphome::smart_signage::led
