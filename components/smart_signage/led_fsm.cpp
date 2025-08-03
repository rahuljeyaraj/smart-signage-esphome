// led_fsm.cpp
#include "led/led_fsm.h"

namespace esphome::smart_signage::led {

/*──────────────────────── Constructor ───────────────────────*/
FSM::FSM(ctrl::Q &q) : ctrlQ_(q) { LOGI("LED FSM created"); }

/*──────────────────────── Guards ────────────────────────────*/
bool FSM::isReadyGuard(const CmdSetup &) {
    LOGI("onSetup: initializing hardware...");
    if (!stubHardwareInit()) {
        LOGE("onSetup: hardware init failed");
        ctrlQ_.post(ctrl::EvtLedError{});
        return false;
    }
    LOGI("onSetup: success");
    ctrlQ_.post(ctrl::EvtLedReady{});
    return true;
}

/*──────────────────────── Top-level actions ─────────────────*/
void FSM::onCmdSetup(const CmdSetup &) {
    LOGI("CmdSetup received (already handled by guard)");
    /* If you need to tell the controller we’re ready, do it here:
       ctrlQ_.post(ctrl::EvtLedReady{}); */
}

void FSM::onCmdTeardown(const CmdTeardown &) {
    LOGI("CmdTeardown → turning LEDs OFF and returning to Idle");
    // hwLedOff();
}

/*──────────────────────── Ready-mode actions ────────────────*/
void FSM::onCmdOn(const CmdOn &cmd) {
    LOGI("CmdOn → brightness=%u%%", static_cast<unsigned>(cmd.brightPct));
    // hwSetBrightness(cmd.brightPct);
}

void FSM::onCmdOff(const CmdOff &) {
    LOGI("CmdOff → LED off");
    // hwLedOff();
}

void FSM::onCmdBreathe(const CmdBreathe &cmd) {
    LOGI(TAG,
        "CmdBreathe → bright=%u%%  fadeIn=%ums  fadeOut=%ums  cycles=%u",
        static_cast<unsigned>(cmd.brightPct),
        static_cast<unsigned>(cmd.fadeInMs),
        static_cast<unsigned>(cmd.fadeOutMs),
        static_cast<unsigned>(cmd.maxCycles));
    /* Example: launch first fade-in and arm a timer/ISR to raise
       EvtFadeInEnd / EvtFadeOutEnd events. */
    // hwStartBreathe(cmd);
}

/*──────────────────────── Breathing loop helpers ───────────*/
void FSM::onFadeInEnd(const EvtFadeEnd &) {
    LOGI("Fade-in finished → start fade-out");
    // hwStartFadeOut();
}

void FSM::onFadeOutEnd(const EvtFadeEnd &) {
    LOGI("Fade-out finished → start fade-in");
    // hwStartFadeIn();
}

/*──────────────────────── Error handler ─────────────────────*/
void FSM::onError() {
    LOGE("Entered Error state!");
    // Optionally notify controller or blink red
}

/*──────────────────────── Hardware stub ─────────────────────*/
bool FSM::stubHardwareInit() {
    /* Replace with real GPIO / PWM init.
       Return true if everything came up OK. */
    LOGI("stubHardwareInit()");
    return true;
}

} // namespace esphome::smart_signage::led
