#include "audio/audio_fsm.h"

namespace esphome::smart_signage::audio {

/*──────────────────────── Ctor ─────────────────────────*/
FSM::FSM(ctrl::Q &q) : ctrlQ_(q) { SS_LOGI("Audio FSM created"); }

/*──────────────────────── Guards ───────────────────────*/
bool FSM::isReadyGuard(const CmdSetup &) {
    SS_LOGI("onSetup: initializing hardware...");
    if (!stubHardwareInit()) {
        SS_LOGE("onSetup: hardware init failed");
        ctrlQ_.post(ctrl::EvtAudioError{});
        return false;
    }
    SS_LOGI("onSetup: success");
    ctrlQ_.post(ctrl::EvtAudioReady{});
    return true;
}

/*──────────────────────── Actions ──────────────────────*/
void FSM::onCmdTeardown(const CmdTeardown &) {
    SS_LOGI("CmdTeardown → shutting down audio subsystem");
    // hwAudioTeardown();
}

void FSM::onCmdPlay(const CmdPlay &cmd) {
    SS_LOGI("CmdPlay → \"%s\" vol=%u%%", cmd.filePath, static_cast<unsigned>(cmd.volPct));
    // hwAudioPlay(cmd.filePath, cmd.volPct);
}

void FSM::onCmdStop(const CmdStop &) {
    SS_LOGI("CmdStop → stopping playback");
    // hwAudioStop();
}

/*──────────────────────── Error handler ─────────────────────*/
void FSM::onError() {
    SS_LOGE("Entered Error state!");
    // Optionally inform controller
}

/*──────────────────────── Hardware stub ─────────────────────*/
bool FSM::stubHardwareInit() {
    /* Replace with real I²S/DAC init; return true on success */
    SS_LOGI("stubHardwareInit()");
    return true;
}

} // namespace esphome::smart_signage::audio
