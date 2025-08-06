#include "ctrl/ctrl_fsm.h"

namespace esphome::smart_signage::ctrl {

/*──────────────────────── Ctor ─────────────────────────*/
FSM::FSM(radar::Q &radarQ, imu::Q &imuQ, led::Q &ledQ, audio::Q &audioQ)
    : radarQ_(radarQ), imuQ_(imuQ), ledQ_(ledQ), audioQ_(audioQ) {}

/*──────────────────────── Guards ───────────────────────*/
bool FSM::guardRadarReady(const EvtRadarReady &) {
    readyBits_.set(static_cast<size_t>(Intf::Radar));
    const bool all = readyBits_.all();
    LOGI("Guard: RadarReady – %u/%u ready → all=%s",
        static_cast<unsigned>(readyBits_.count()),
        static_cast<unsigned>(kIntfCnt),
        all ? "YES" : "NO");
    return all;
}

bool FSM::guardImuReady(const EvtImuReady &) {
    readyBits_.set(static_cast<size_t>(Intf::Imu));
    const bool all = readyBits_.all();
    LOGI("Guard: ImuReady   – %u/%u ready → all=%s",
        static_cast<unsigned>(readyBits_.count()),
        static_cast<unsigned>(kIntfCnt),
        all ? "YES" : "NO");
    return all;
}

bool FSM::guardLedReady(const EvtLedReady &) {
    readyBits_.set(static_cast<size_t>(Intf::Led));
    const bool all = readyBits_.all();
    LOGI("Guard: LedReady   – %u/%u ready → all=%s",
        static_cast<unsigned>(readyBits_.count()),
        static_cast<unsigned>(kIntfCnt),
        all ? "YES" : "NO");
    return all;
}

bool FSM::guardAudioReady(const EvtAudioReady &) {
    readyBits_.set(static_cast<size_t>(Intf::Audio));
    const bool all = readyBits_.all();
    LOGI("Guard: AudioReady – %u/%u ready → all=%s",
        static_cast<unsigned>(readyBits_.count()),
        static_cast<unsigned>(kIntfCnt),
        all ? "YES" : "NO");
    return all;
}

/*──────────────────────── Actions ──────────────────────*/
void FSM::onCmdSetup(const CmdSetup &) {
    LOGI("Action: onCmdSetup -> broadcast CmdSetup");
    readyBits_.reset(); // fresh round
    radarQ_.post(radar::CmdSetup{});
    imuQ_.post(imu::CmdSetup{});
    ledQ_.post(led::CmdSetup{});
    audioQ_.post(audio::CmdSetup{});
}

void FSM::onCmdStart(const CmdStart &e) {
    LOGI("Action: onCmdStart -> broadcast CmdStart (runTimeMins=%u)", e.runTimeMins);
    runTimeMins_ = e.runTimeMins;
    radarQ_.post(radar::CmdStart{});
    imuQ_.post(imu::CmdStart{});
}

void FSM::onCmdStop(const CmdStop &) {
    LOGI("Action: onCmdStop -> broadcast CmdStop");
    radarQ_.post(radar::CmdStop{});
    imuQ_.post(imu::CmdStop{});
}

void FSM::onCmdTeardown(const CmdTeardown &) {
    LOGI("Action: onCmdTeardown -> broadcast CmdTeardown");
    radarQ_.post(radar::CmdTeardown{});
    imuQ_.post(imu::CmdTeardown{});
    ledQ_.post(led::CmdTeardown{});
    audioQ_.post(audio::CmdTeardown{});
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

void FSM::onSessionEnd(const EvtTimeout &) {
    LOGI("Timeout: Active phase timed out – stopping & tearing down all interfaces");
    onCmdStop({});
    onCmdTeardown({});
}

void FSM::onError() { LOGE("Entered Error state!"); }

} // namespace esphome::smart_signage::ctrl
