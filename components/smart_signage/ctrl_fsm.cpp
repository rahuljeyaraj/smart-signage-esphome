#include "ctrl/ctrl_fsm.h"

namespace esphome::smart_signage::ctrl {

/*──────────────────────── Ctor ─────────────────────────*/
FSM::FSM(radar::Q &radarQ, imu::Q &imuQ, led::Q &ledQ, audio::Q &audioQ, timer::ITimer &timer,
    ProfilesConfigT &cfg, UserIntfT &ui)
    : radarQ_(radarQ), imuQ_(imuQ), ledQ_(ledQ), audioQ_(audioQ), timer_(timer), cfg_(cfg),
      ui_(ui) {}

/*──────────────────────── Guards ───────────────────────*/
bool FSM::guardRadarReady(const EvtRadarReady &) {
    readyBits_.set(static_cast<size_t>(Intf::Radar));
    const bool all = readyBits_.all();
    SS_LOGI("Guard: RadarReady - %u/%u ready → all=%s",
        static_cast<unsigned>(readyBits_.count()),
        static_cast<unsigned>(kIntfCnt),
        all ? "YES" : "NO");
    return all;
}

bool FSM::guardImuReady(const EvtImuReady &) {
    readyBits_.set(static_cast<size_t>(Intf::Imu));
    const bool all = readyBits_.all();
    SS_LOGI("Guard: ImuReady   - %u/%u ready → all=%s",
        static_cast<unsigned>(readyBits_.count()),
        static_cast<unsigned>(kIntfCnt),
        all ? "YES" : "NO");
    return all;
}

bool FSM::guardLedReady(const EvtLedReady &) {
    readyBits_.set(static_cast<size_t>(Intf::Led));
    const bool all = readyBits_.all();
    SS_LOGI("Guard: LedReady   - %u/%u ready → all=%s",
        static_cast<unsigned>(readyBits_.count()),
        static_cast<unsigned>(kIntfCnt),
        all ? "YES" : "NO");
    return all;
}

bool FSM::guardAudioReady(const EvtAudioReady &) {
    readyBits_.set(static_cast<size_t>(Intf::Audio));
    const bool all = readyBits_.all();
    SS_LOGI("Guard: AudioReady - %u/%u ready → all=%s",
        static_cast<unsigned>(readyBits_.count()),
        static_cast<unsigned>(kIntfCnt),
        all ? "YES" : "NO");
    return all;
}

/*──────────────────────── Actions ──────────────────────*/
void FSM::onCmdSetup(const CmdSetup &) {
    SS_LOGI("Action: onCmdSetup -> broadcast CmdSetup");

    //  Build the profile list from cfg_ and set UI options
    etl::vector<ProfileName, SS_MAX_PROFILES> profiles;
    cfg_.getProfileList(profiles);
    ui_.setProfileOptions(profiles);

    // Load current profile (or default) from NVS, then publish its values to UI
    ProfileName   currName;
    ProfileValues vals{};
    NvsSmartSignage::loadCurrentOrDefault(profiles[0], currName, vals);

    // Push selection + numbers into the UI (no echo back)
    ui_.setCurrentProfile(currName);
    ui_.setSessionMins(vals.sessionMins);
    ui_.setRadarRangeCm(vals.radarRangeCm);
    ui_.setAudioVolPct(vals.audioVolPct);
    ui_.setLedBrightPct(vals.ledBrightPct);

    readyBits_.reset();
    radarQ_.post(radar::CmdSetup{});
    imuQ_.post(imu::CmdSetup{});
    ledQ_.post(led::CmdSetup{});
    audioQ_.post(audio::CmdSetup{});
}

void FSM::onCmdStart(const CmdStart &) {
    ProfileValues vals{};
    NvsSmartSignage::loadCurrentVal(vals);
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
        static_cast<unsigned>(e.distanceCm),
        static_cast<unsigned>(e.timestampTicks));
}

void FSM::onEvtImuFell(const EvtImuFell &) { SS_LOGI("Event: IMU reports a fall detected!"); }
void FSM::onEvtImuRose(const EvtImuRose &) {
    SS_LOGI("Event: IMU reports device has been restored (rose)");
}

void FSM::onSetupTimeout() {
    SS_LOGI("Timeout: Setup phase timed out! Transitioning to Error state");
}

void FSM::onSessionEnd() {
    SS_LOGI("Timeout: Active phase timed out - stopping & tearing down all interfaces");
    onCmdStop({});
    onCmdTeardown({});
}

void FSM::onUiProfileUpdate(const EvtUiProfileUpdate &e) {
    SS_LOGI("onUiProfileUpdate -> \"%s\"", e.profileName.c_str());
    // Persist current profile name
    if (!e.profileName.empty()) {
        NvsSmartSignage::setCurrentProfile(e.profileName);
        SS_LOGI("UI: Active profile -> \"%s\"", e.profileName.c_str());
    } else {
        SS_LOGW("UI: Received empty profile name; ignoring");
        return;
    }

    // Load values for the new profile (created with defaults if missing) and push to UI
    ProfileValues vals{};
    if (!NvsSmartSignage::loadProfile(e.profileName, vals)) {
        SS_LOGE("UI: Failed to load profile \"%s\" from NVS", e.profileName.c_str());
        return;
    }
    // Update UI numbers (suppressed echo in UserIntf)
    ui_.setSessionMins(vals.sessionMins);
    ui_.setRadarRangeCm(vals.radarRangeCm);
    ui_.setAudioVolPct(vals.audioVolPct);
    ui_.setLedBrightPct(vals.ledBrightPct);
}

void FSM::onUiSessionMinsUpdate(const EvtUiSessionMinsUpdate &e) {
    ProfileName   name = resolveActiveProfile_();
    ProfileValues vals{};
    if (!name.empty()) NvsSmartSignage::loadProfile(name, vals);
    vals.sessionMins = e.mins;
    if (!name.empty()) NvsSmartSignage::saveProfile(name, vals);
    SS_LOGI("UI: sessionMins -> %u (profile=\"%s\")", (unsigned) e.mins, name.c_str());
}

void FSM::onUiRangeCmUpdate(const EvtUiRangeCmUpdate &e) {
    ProfileName   name = resolveActiveProfile_();
    ProfileValues vals{};
    if (!name.empty()) NvsSmartSignage::loadProfile(name, vals);
    vals.radarRangeCm = e.cm;
    if (!name.empty()) NvsSmartSignage::saveProfile(name, vals);
    SS_LOGI("UI: radarRangeCm -> %u (profile=\"%s\")", (unsigned) e.cm, name.c_str());
}

void FSM::onUiAudioVolUpdate(const EvtUiAudioVolUpdate &e) {
    ProfileName   name = resolveActiveProfile_();
    ProfileValues vals{};
    if (!name.empty()) NvsSmartSignage::loadProfile(name, vals);
    vals.audioVolPct = e.pct;
    if (!name.empty()) NvsSmartSignage::saveProfile(name, vals);
    SS_LOGI("UI: audioVolPct -> %hhu (profile=\"%s\")", e.pct, name.c_str());
}

void FSM::onUiLedBrightUpdate(const EvtUiLedBrightUpdate &e) {
    ProfileName   name = resolveActiveProfile_();
    ProfileValues vals{};
    if (!name.empty()) NvsSmartSignage::loadProfile(name, vals);
    vals.ledBrightPct = e.pct;
    if (!name.empty()) NvsSmartSignage::saveProfile(name, vals);
    SS_LOGI("UI: ledBrightPct -> %hhu (profile=\"%s\")", e.pct, name.c_str());
}

void FSM::onError() { SS_LOGE("Entered Error state!"); }

ProfileName FSM::resolveActiveProfile_() {
    // Try NVS current profile
    ProfileName curr;
    NvsSmartSignage::getCurrentProfile(curr); // may leave curr empty if missing
    // If missing in NVS, fall back to first configured profile and set it current
    if (curr.empty()) {
        etl::vector<ProfileName, SS_MAX_PROFILES> profiles;
        cfg_.getProfileList(profiles);
        if (!profiles.empty()) {
            curr = profiles[0];
            NvsSmartSignage::setCurrentProfile(curr);
            SS_LOGW("Active profile absent in NVS, defaulting to \"%s\"", curr.c_str());
        }
    }
    return curr;
}

} // namespace esphome::smart_signage::ctrl
