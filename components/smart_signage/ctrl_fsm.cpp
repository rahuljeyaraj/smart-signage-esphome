#include "ctrl/ctrl_fsm.h"

namespace esphome::smart_signage::ctrl {

FSM::FSM(radar::Q &radarQ) : radarQ_(radarQ) {}

// Guards
bool FSM::ReadyGuard(const EvtRadarReady &e) {
    LOGI(TAG, "ReadyGuard: Received EvtRadarReady (radar is ready)");
    // In future: track readiness of all required modules/interfaces.
    return true;
}

// Actions
void FSM::onCmdSetup(const CmdSetup &) {
    LOGI(TAG, "Action: onCmdSetup -> Sending radar setup command");
    radarQ_.post(radar::CmdSetup{});
}

void FSM::onCmdStart(const CmdStart &e) {
    LOGI(TAG, "Action: onCmdStart -> Starting system (runTimeMins=%u)", e.runTimeMins);
    radarQ_.post(radar::CmdStart{});
    runTimeMins_ = e.runTimeMins;
    // Start system runtime timer (to be implemented as needed)
}

void FSM::onCmdStop(const CmdStop &) {
    LOGI(TAG, "Action: onCmdStop -> Stopping radar");
    radarQ_.post(radar::CmdStop{});
}

void FSM::onCmdTeardown(const CmdTeardown &) {
    LOGI(TAG, "Action: onCmdTeardown -> Tearing down radar and resetting to Idle");
    radarQ_.post(radar::CmdTeardown{});
}

void FSM::onEvtRadarData(const EvtRadarData &e) {
    LOGI(TAG,
        "Data: Radar detected=%s, distance=%u cm, timestamp=%u",
        e.detected ? "YES" : "NO",
        static_cast<unsigned>(e.distanceCm),
        static_cast<unsigned>(e.timestampTicks));
}

void FSM::onEvtFell(const EvtFell &) { LOGI(TAG, "Event: IMU reports a fall detected!"); }

void FSM::onEvtRose(const EvtRose &) {
    LOGI(TAG, "Event: IMU reports device has been restored (rose)");
}

void FSM::onSetupTimeout(const EvtTimeout &) {
    LOGI(TAG, "Timeout: Setup phase timed out! Transitioning to Error state");
}

void FSM::onActiveTimeout(const EvtTimeout &) {
    LOGI(TAG, "Timeout: Active phase timed out, sending stop and teardown to radar");
    radarQ_.post(radar::CmdStop{});
    radarQ_.post(radar::CmdTeardown{});
}

void FSM::onError() { LOGE(TAG, "Entered Error state!"); }

} // namespace esphome::smart_signage::ctrl
