#include "radar/radar_fsm.h"
#include "log.h"
#include <Arduino.h>

namespace esphome::smart_signage::radar {

FSM::FSM(ctrl::Q &q, hal::IRadarHal &hal, timer::ITimer &t) : ctrlQ_(q), hal_(hal), timer_(t) {}

bool FSM::isReadyGuard(const CmdSetup &e) {

    SS_LOGI("onSetup: Initializing hardware...");
    if (!hal_.init()) {
        SS_LOGE("onSetup: HAL init failed");
        ctrlQ_.post(ctrl::EvtRadarError{});
        return false;
    }
    SS_LOGI("onSetup: success");
    ctrlQ_.post(ctrl::EvtRadarReady{});
    return true;
}

bool FSM::hasDataGuard() {
    hasRawData_    = false;
    rawDetected_   = false;
    rawDistanceCm_ = 0;

    int flushCnt = 0;
    for (; flushCnt < kMaxFlushCnt && hal_.hasNewData(); ++flushCnt) {
        rawDistanceCm_ = hal_.getDistance();
        rawDetected_   = hal_.presenceDetected();
        hasRawData_    = true;
    }
    if (flushCnt == kMaxFlushCnt) {
        SS_LOGW("Radar flush: max flush count %d reached", kMaxFlushCnt);
    }
    return hasRawData_;
}

bool FSM::detectGuard() {
    bool     detected   = false;
    uint16_t distanceCm = 0;
    if (hasRawData_) {
        // Apply kalman filter
        distanceCm = static_cast<uint16_t>(filter_.updateEstimate(rawDistanceCm_) + 0.5f);
        detected   = rawDetected_ && (distanceCm <= detDistCm_);
    }

    if (detected) {
        SS_LOGD(
            "raw=%u filtered=%u detected=%s", rawDistanceCm_, distanceCm, detected ? "YES" : "NO");
        ctrlQ_.post(ctrl::EvtRadarDistance{distanceCm, xTaskGetTickCount()});
    }
    return detected;
}

void FSM::onCmdStart(const CmdStart &) {
    SS_LOGD("onStart: radar polling at %u ms", sampleIntMs_);
    // Starts the timer
    timer_.startPeriodic(uint64_t(sampleIntMs_) * 1000ULL);
}

void FSM::onCmdStop(const CmdStop &) { timer_.stop(); }

void FSM::onCmdTeardown(const CmdTeardown &) { timer_.stop(); }

// void FSM::onEvtTimerPoll(const EvtTimerPoll &) {
//     bool     hasData = false;
//     uint16_t raw     = 0;
//     bool     pres    = false;

//     int flushCnt = 0;
//     for (; flushCnt < kMaxFlushCnt && hal_.hasNewData(); ++flushCnt) {
//         raw     = hal_.getDistance();
//         pres    = hal_.presenceDetected();
//         hasData = true;
//     }
//     if (flushCnt == kMaxFlushCnt) {
//         SS_LOGW("Radar flush: max flush count %d reached", kMaxFlushCnt);
//     }
//     if (!hasData) { return; }

//     auto filt   = static_cast<uint16_t>(filter_.updateEstimate(raw) + 0.5f);
//     bool detect = pres && (filt <= detDistCm_);
//     // Serial.printf(">raw:%u,filt:%u\r\n", raw, filt);
// }

void FSM::onSetDist(const SetRangeCm &c) {
    detDistCm_ = c.cm;
    SS_LOGI("onSetDist: max distance = %u cm", detDistCm_);
}

void FSM::onSetSampleInt(const SetSampleInt &c) {
    sampleIntMs_ = c.ms;
    SS_LOGI("onSetSampleInt: interval = %u ms", sampleIntMs_);
}

void FSM::onDetectedEntry() {
    SS_LOGI("Is detected");
    ctrlQ_.post(ctrl::EvtRadarDetected{}, portMAX_DELAY); // Blocking
}

void FSM::onDetectedExit() {
    SS_LOGI("Is clear");
    ctrlQ_.post(ctrl::EvtRadarClear{}, portMAX_DELAY); // Blocking
}
void FSM::onError() { SS_LOGE("Entered Error state!"); }

} // namespace esphome::smart_signage::radar
