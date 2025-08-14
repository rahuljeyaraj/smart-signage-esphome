#include "ctrl/ctrl_fsm.h"

namespace esphome::smart_signage::ctrl {

/*──────────────────────── Ctor ─────────────────────────*/
FSM::FSM(radar::Q &radarQ, imu::Q &imuQ, led::Q &ledQ, audio::Q &audioQ, timer::ITimer &timer,
    ProfileSettings &profiles, UserIntfT &ui)
    : radarQ_(radarQ), imuQ_(imuQ), ledQ_(ledQ), audioQ_(audioQ), timer_(timer),
      profiles_(profiles), ui_(ui) {}

/*──────────────────────── Guards ───────────────────────*/
bool FSM::guardRadarReady(const EvtRadarReady &) {
    readyBits_.set(static_cast<size_t>(Intf::Radar));
    const bool all = readyBits_.all();
    SS_LOGI("Guard: RadarReady - %u/%u ready -> %s",
        (unsigned) readyBits_.count(),
        (unsigned) kIntfCnt,
        all ? "ALL" : "NOT ALL");
    return all;
}

bool FSM::guardImuReady(const EvtImuReady &) {
    readyBits_.set(static_cast<size_t>(Intf::Imu));
    const bool all = readyBits_.all();
    SS_LOGI("Guard: ImuReady   - %u/%u ready -> %s",
        (unsigned) readyBits_.count(),
        (unsigned) kIntfCnt,
        all ? "ALL" : "NOT ALL");
    return all;
}

bool FSM::guardLedReady(const EvtLedReady &) {
    readyBits_.set(static_cast<size_t>(Intf::Led));
    const bool all = readyBits_.all();
    SS_LOGI("Guard: LedReady   - %u/%u ready -> %s",
        (unsigned) readyBits_.count(),
        (unsigned) kIntfCnt,
        all ? "ALL" : "NOT ALL");
    return all;
}

bool FSM::guardAudioReady(const EvtAudioReady &) {
    readyBits_.set(static_cast<size_t>(Intf::Audio));
    const bool all = readyBits_.all();
    SS_LOGI("Guard: AudioReady - %u/%u ready -> %s",
        (unsigned) readyBits_.count(),
        (unsigned) kIntfCnt,
        all ? "ALL" : "NOT ALL");
    return all;
}

/*──────────────────────── Actions ──────────────────────*/
void FSM::onCmdSetup(const CmdSetup &) {
    SS_LOGI("Action: onCmdSetup -> broadcast CmdSetup");

    // // Build the profile list from ProfileSettings and push to UI
    // etl::vector<ProfileName, SS_MAX_PROFILES> profiles;
    // profiles_.getProfileNames(profiles);
    // ui_.setProfileOptions(profiles);

    // // Load current (or default=first) and publish its values
    // ProfileName   currName;
    // ProfileValues vals{};
    // profiles_.loadCurrentOrDefault(profiles.empty() ? ProfileName{} : profiles[0], currName,
    // vals);

    // ui_.setCurrentProfile(currName);
    // ui_.setSessionMins(vals.sessionMins);
    // ui_.setRadarRangeCm(vals.radarRangeCm);
    // ui_.setAudioVolPct(vals.audioVolPct);
    // ui_.setLedBrightPct(vals.ledBrightPct);

    // // Kick sub-systems to setup
    // readyBits_.reset();
    // radarQ_.post(radar::CmdSetup{});
    // imuQ_.post(imu::CmdSetup{});
    // ledQ_.post(led::CmdSetup{});
    // audioQ_.post(audio::CmdSetup{});
}

void FSM::onCmdStart(const CmdStart &) {
    ProfileValues vals{};
    profiles_.loadCurrentVal(vals);
    SS_LOGI("Action: onCmdStart -> broadcast CmdStart (sessionMins=%u)", vals.sessionMins);
    sessionMins_ = vals.sessionMins;
    radarQ_.post(radar::CmdStart{});
    imuQ_.post(imu::CmdStart{});
}

void FSM::onCmdStop(const CmdStop &) {
    SS_LOGI("Action: onCmdStop -> broadcast CmdStop");
    radarQ_.post(radar::CmdStop{});
    imuQ_.post(imu::CmdStop{});
}

void FSM::onCmdTeardown(const CmdTeardown &) {
    SS_LOGI("Action: onCmdTeardown -> broadcast CmdTeardown");
    radarQ_.post(radar::CmdTeardown{});
    imuQ_.post(imu::CmdTeardown{});
    ledQ_.post(led::CmdTeardown{});
    audioQ_.post(audio::CmdTeardown{});
}

void FSM::onEvtRadarData(const EvtRadarData &e) {
    SS_LOGI("Data: Radar detected=%s, distance=%u cm, timestamp=%u",
        e.detected ? "YES" : "NO",
        (unsigned) e.distanceCm,
        (unsigned) e.timestampTicks);
}

void FSM::onEvtImuFell(const EvtImuFell &) { SS_LOGI("Event: IMU reports a fall detected!"); }
void FSM::onEvtImuRose(const EvtImuRose &) { SS_LOGI("Event: IMU reports device restored"); }

void FSM::onSetupTimeout() { SS_LOGI("Timeout: Setup phase timed out"); }

void FSM::onSessionEnd() {
    SS_LOGI("Timeout: Active phase timed out - stopping & tearing down all interfaces");
    onCmdStop({});
    onCmdTeardown({});
}

/* UI echoes handled later; leaving declarations in header for now */
void FSM::onUiProfileUpdate(const EvtUiProfileUpdate &) {}
void FSM::onUiSessionMinsUpdate(const EvtUiSessionMinsUpdate &) {}
void FSM::onUiRangeCmUpdate(const EvtUiRangeCmUpdate &) {}
void FSM::onUiAudioVolUpdate(const EvtUiAudioVolUpdate &) {}
void FSM::onUiLedBrightUpdate(const EvtUiLedBrightUpdate &) {}

void FSM::onError() { SS_LOGE("Entered Error state"); }

ProfileName FSM::resolveActiveProfile_() {
    ProfileName name;
    // profiles_.getCurrentProfile(name);
    return name;
}

} // namespace esphome::smart_signage::ctrl
