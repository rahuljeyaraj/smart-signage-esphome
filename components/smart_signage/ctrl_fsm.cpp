#include "ctrl/ctrl_fsm.h"

namespace esphome::smart_signage::ctrl {

FSM::FSM(radar::Q &radarQ, imu::Q &imuQ) : radarQ_(radarQ), imuQ_(imuQ) {}

// Guards
bool FSM::isAllReadyGuard(const EvtRadarReady &e) {
    LOGI("isAllReadyGuard: Received EvtRadarReady (radar is ready)");
    // In future: track readiness of all required modules/interfaces.
    return true;
}

// Actions
void FSM::onCmdSetup(const CmdSetup &) {
    LOGI("Action: onCmdSetup -> Sending radar setup command");
    radarQ_.post(radar::CmdSetup{});
    imuQ_.post(imu::CmdSetup{});
}

void FSM::onCmdStart(const CmdStart &e) {
    LOGI("Action: onCmdStart -> Starting system (runTimeMins=%u)", e.runTimeMins);
    runTimeMins_ = e.runTimeMins;
    radarQ_.post(radar::CmdStart{});
    imuQ_.post(imu::CmdStart{});

    // Start system runtime timer (to be implemented as needed)
}

void FSM::onCmdStop(const CmdStop &) {
    LOGI("Action: onCmdStop -> Stopping radar");
    radarQ_.post(radar::CmdStop{});
    imuQ_.post(imu::CmdStop{});
}

void FSM::onCmdTeardown(const CmdTeardown &) {
    LOGI("Action: onCmdTeardown -> Tearing down radar and resetting to Idle");
    radarQ_.post(radar::CmdTeardown{});
    imuQ_.post(imu::CmdTeardown{});
}

void FSM::onEvtRadarData(const EvtRadarData &e) {
    LOGI(TAG,
        "Data: Radar detected=%s, distance=%u cm, timestamp=%u",
        e.detected ? "YES" : "NO",
        static_cast<unsigned>(e.distanceCm),
        static_cast<unsigned>(e.timestampTicks));
}

void FSM::onEvtImuFell(const EvtImuFell &) { LOGI("Event: IMU reports a fall detected!"); }

void FSM::onEvtImuRose(const EvtImuRose &) {
    LOGI("Event: IMU reports device has been restored (rose)");
}

void FSM::onSetupTimeout(const EvtTimeout &) {
    LOGI("Timeout: Setup phase timed out! Transitioning to Error state");
}

void FSM::onActiveTimeout(const EvtTimeout &) {
    LOGI("Timeout: Active phase timed out, sending stop and teardown to radar");
    radarQ_.post(radar::CmdStop{});
    radarQ_.post(radar::CmdTeardown{});
    imuQ_.post(imu::CmdStop{});
    imuQ_.post(imu::CmdTeardown{});
}

void FSM::onError() { LOGE("Entered Error state!"); }

} // namespace esphome::smart_signage::ctrl
