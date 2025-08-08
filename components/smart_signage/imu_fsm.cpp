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
    Vector acc      = hal_.getAccel();
    double tiltDeg  = computeTiltAngle(acc, refAcc_);
    bool   isFallen = tiltDeg >= fallAngleDeg_;
    isFallenDebounce_.add(isFallen);
    // if (isFallenDebounce_.has_changed()) {
    //     LOG_I("Fallen:%s", isFallenDebounce_.is_set() ? "true" : "false");
    //     if (isFallenDebounce_.is_set()) { return true }
    //     return false;
    // }
    return isFallenDebounce_.is_set();
}

// Actions
void FSM::onCmdStart(const CmdStart &e) {
    LOGD("onStart: imu polling at %u ms", sampleIntMs_);
    // Save reference acceration vector
    refAcc_ = hal_.getAccel();
    // Clear isfallen state
    isFallenDebounce_.add(false);
    // Starts the timer
    // timer_.startPeriodic(uint64_t(sampleIntMs_) * 1000ULL);
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

void FSM::onFallenEntry() { LOGE("Entered Fallen state"); }

void FSM::onFallenExit() { LOGE("Exiting Fallen state"); }

void FSM::onError() { LOGE("Entered Error state"); }

double FSM::computeTiltAngle(const Vector &curAccel, const Vector &refAccel) const {
    double dotProd = curAccel.dot(refAccel);
    double magProd = curAccel.norm() * refAccel.norm();
    if (magProd <= 0.0) return 0.0;

    double cosθ = dotProd / magProd;
    cosθ        = std::clamp(cosθ, -1.0, 1.0);

    return std::acos(cosθ) * 180.0 / M_PI;
}

} // namespace esphome::smart_signage::imu
