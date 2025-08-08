#include <ArduinoEigenDense.h>
#include "imu/imu_fsm.h"
#include "log.h"

namespace esphome::smart_signage::imu {

constexpr char FSM::TAG[];

FSM::FSM(ctrl::Q &q, hal::IImuHal &hal, timer::ITimer &t) : ctrlQ_(q), hal_(hal), timer_(t) {}

// Guard:
bool FSM::isReadyGuard(const CmdSetup &e) {
    LOGI("onSetup: initializing hardware...");
    if (!hal_.init()) {
        LOGE("onSetup: hardware init failed");
        ctrlQ_.post(ctrl::EvtImuError{});
        return false;
    }
    LOGI("onSetup: success");
    ctrlQ_.post(ctrl::EvtImuReady{});
    return true;
}

bool FSM::isFallenGuard(const EvtTimerPoll &e) {
    hal_.read();
    Vector   acc      = hal_.getAccel();
    uint16_t tiltDeg  = computeTiltAngle(acc, refAcc_);
    bool     isFallen = tiltDeg >= fallAngleDeg_;
    LOGD("fall angle: %u, curr angle: %u ", fallAngleDeg_, tiltDeg);
    isFallenDebounce_.add(isFallen);
    return isFallenDebounce_.is_set();
}

// Actions
void FSM::onCmdStart(const CmdStart &e) {
    LOGD("onStart: imu polling at %u ms", sampleIntMs_);
    hal_.read();
    refAcc_ = hal_.getAccel();                              // Save reference acceration vector
    isFallenDebounce_.add(false);                           // Clear isfallen state
    timer_.startPeriodic(uint64_t(sampleIntMs_) * 1000ULL); // Starts the timer
}

void FSM::onCmdStop(const CmdStop &e) { timer_.stop(); }

void FSM::onCmdTeardown(const CmdTeardown &e) {
    LOGI("onCmdTeardown()");
    timer_.stop();
}

void FSM::onSetFallAngle(const SetFallAngle &e) {
    fallAngleDeg_ = e.deg;
    LOGI("onSetFallAngle(): %u°", fallAngleDeg_);
}

void FSM::onSetConfirmCnt(const SetConfirmCnt &e) {
    confirmCnt_ = e.cnt;
    LOGI("onSetConfirmCnt(): %u", confirmCnt_);
}

void FSM::onSetSampleInt(const SetSampleInt &e) {
    sampleIntMs_ = e.ms;
    LOGI("onSetSampleInt(): %u ms", sampleIntMs_);
}

void FSM::onFallenEntry() {
    LOGI("Is fallen");
    ctrlQ_.post(ctrl::EvtImuFell{}, portMAX_DELAY); // Blocking
}

void FSM::onFallenExit() {
    LOGI("Is upright");
    ctrlQ_.post(ctrl::EvtImuRose{}, portMAX_DELAY); // Blocking
}

void FSM::onError() { LOGE("Entered Error state"); }

// Helper
uint16_t FSM::computeTiltAngle(const Vector &curAccel, const Vector &refAccel) const {
    double dotProd = curAccel.dot(refAccel);
    double magProd = curAccel.norm() * refAccel.norm();
    if (magProd <= 0.0) return 0.0;
    double cosθ = dotProd / magProd;
    cosθ        = std::clamp(cosθ, -1.0, 1.0);
    return static_cast<uint16_t>(std::lround(std::acos(cosθ) * 180.0 / M_PI));
}

} // namespace esphome::smart_signage::imu
