#include "imu/imu_fsm.h"
#include "log.h"

namespace esphome::smart_signage::imu {

constexpr char FSM::TAG[];

FSM::FSM(ctrl::Q &q) : ctrlQ_(q) { LOGI(TAG, "IMU FSM constructed"); }

// Guard:
bool FSM::isReadyGuard(const CmdSetup &e) {
    LOGI("onSetup: initializing hardware...");
    if (!stubHardwareInit()) {
        LOGE("onSetup: hardware init failed");
        ctrlQ_.post(ctrl::EvtImuError{});
        return false;
    }
    LOGI("onSetup: success");
    ctrlQ_.post(ctrl::EvtImuReady{});
    return true;
}

bool FSM::isFallenGuard(const EvtTimerPoll &e) {
    LOGD(TAG, "isFallenGuard()");
    ctrlQ_.post(ctrl::EvtImuFell{});
    ctrlQ_.post(ctrl::EvtImuRose{});
    return false;
}

// Actions
void FSM::onCmdStart(const CmdStart &e) {
    LOGI(TAG, "onCmdStart()");
    // TODO: start timer, enable polling, etc.
}

void FSM::onCmdStop(const CmdStop &e) {
    LOGI(TAG, "onCmdStop()");
    // TODO: disable polling, stop timers, etc.
}

void FSM::onCmdTeardown(const CmdTeardown &e) {
    LOGI(TAG, "onCmdTeardown()");
    // TODO: clean up, reset state, etc.
}

void FSM::onSetFallAngle(const SetFallAngle &e) {
    fallAngleDeg_ = e.deg;
    LOGI(TAG, "onSetFallAngle(): %u°", fallAngleDeg_);
}

void FSM::onSetConfirmCnt(const SetConfirmCnt &e) {
    confirmCnt_ = e.cnt;
    LOGI(TAG, "onSetConfirmCnt(): %u", confirmCnt_);
}

void FSM::onSetSampleInt(const SetSampleInt &e) {
    sampleIntMs_ = e.ms;
    LOGI(TAG, "onSetSampleInt(): %u ms", sampleIntMs_);
}

void FSM::onFallenEntry() { LOGE(TAG, "Entered Fallen state"); }

void FSM::onFallenExit() { LOGE(TAG, "Exiting Fallen state"); }

void FSM::onError() { LOGE(TAG, "Entered Error state"); }

// Stub for hardware initialization
bool FSM::stubHardwareInit() {
    LOGD(TAG, "stubHardwareInit()");
    // TODO: perform real hardware init here
    return true;
}

} // namespace esphome::smart_signage::imu
