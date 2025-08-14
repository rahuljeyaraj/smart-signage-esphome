#include "ctrl/ctrl_fsm.h"

namespace esphome::smart_signage::ctrl {

/*──────────────────────── Ctor ─────────────────────────*/
FSM::FSM(radar::Q &radarQ, imu::Q &imuQ, led::Q &ledQ, audio::Q &audioQ, timer::ITimer &timer,
    profile::ProfileCatalog &catalog, profile::ProfileSettings &settings, UserIntfT &ui)
    : radarQ_(radarQ), imuQ_(imuQ), ledQ_(ledQ), audioQ_(audioQ), timer_(timer), catalog_(catalog),
      settings_(settings), ui_(ui) {}

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
    SS_LOGI("Action: onCmdSetup");
    SS_LOGI("Setup dashboard");
    ProfileNames names;
    ProfileName  curr;

    catalog_.getProfileNames(names);
    if (!hasValidCurrProfile(names, curr)) {
        getDefaultCurrProfile(curr);
        settings_.writeCurrentProfile(curr);
    }
    ui_.setProfileOptions(names);
    ui_.setCurrentProfile(curr);
    updateValuesToUi(curr);

    SS_LOGI("broadcast CmdSetup");
    readyBits_.reset();
    radarQ_.post(radar::CmdSetup{});
    imuQ_.post(imu::CmdSetup{});
    ledQ_.post(led::CmdSetup{});
    audioQ_.post(audio::CmdSetup{});
}

void FSM::onCmdStart(const CmdStart &e) {
    SS_LOGI("Action: onCmdStart -> broadcast CmdStart (sessionMins=%u)", e.sessionMins);
    sessionMins_ = e.sessionMins;
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
    ProfileName   curr = e.profileName;
    ProfileValues values{};
    settings_.writeCurrentProfile(curr);
    updateValuesToUi(curr);
}
void FSM::onUiSessionMinsUpdate(const EvtUiSessionMinsUpdate &e) {
    ProfileName   curr;
    ProfileValues values;
    settings_.readCurrentProfile(curr);
    settings_.readProfileValues(curr, values);
    values.sessionMins = e.mins;
    settings_.writeProfileValues(curr, values);
}
void FSM::onUiRangeCmUpdate(const EvtUiRangeCmUpdate &e) {
    ProfileName   curr;
    ProfileValues values;
    settings_.readCurrentProfile(curr);
    settings_.readProfileValues(curr, values);
    values.radarRangeCm = e.cm;
    settings_.writeProfileValues(curr, values);
}
void FSM::onUiAudioVolUpdate(const EvtUiAudioVolUpdate &e) {
    ProfileName   curr;
    ProfileValues values;
    settings_.readCurrentProfile(curr);
    settings_.readProfileValues(curr, values);
    values.audioVolPct = e.pct;
    settings_.writeProfileValues(curr, values);
}
void FSM::onUiLedBrightUpdate(const EvtUiLedBrightUpdate &e) {
    ProfileName   curr;
    ProfileValues values;
    settings_.readCurrentProfile(curr);
    settings_.readProfileValues(curr, values);
    values.ledBrightPct = e.pct;
    settings_.writeProfileValues(curr, values);
}

void FSM::onReady() {
    SS_LOGI("Entered Ready state");
    ProfileName          curr{};
    profile::EventId     ev = profile::EventId::SetupDone;
    audio::AudioPlaySpec audioPlaySpec;
    led::LedPlaySpec     ledPlaySpec;
    settings_.readCurrentProfile(curr);
    if (catalog_.getAudioPlaySpec(curr, ev, audioPlaySpec)) {
        SS_LOGI("Playing audio");
        audioQ_.post(audio::CmdPlay(audioPlaySpec));
    }
    if (catalog_.getLedPlaySpec(curr, ev, ledPlaySpec)) {
        SS_LOGI(
            "Playing led:%d, %d, %d", ledPlaySpec.cnt, ledPlaySpec.pattern, ledPlaySpec.periodMs);
    }
}

void FSM::onError() { SS_LOGE("Entered Error state!"); }

bool FSM::hasValidCurrProfile(ProfileNames &names, ProfileName &nameOut) {
    ProfileName curr{};
    if (!settings_.readCurrentProfile(curr)) {
        SS_LOGE("readCurrentProfile failed");
        return false;
    }

    for (const auto &n : names) {
        if (n == curr) {
            nameOut = curr;
            SS_LOGI("has valid current profile: \"%s\"", curr.c_str());
            return true;
        }
    }

    SS_LOGW("invalid current profile: \"%s\" ", curr.c_str());
    return false;
}

void FSM::getDefaultCurrProfile(ProfileName &defultProfile) {
    ProfileNames names;
    catalog_.getProfileNames(names);
    defultProfile = names[0];

    SS_LOGI("Setting default profile: \"%s\" ", defultProfile.c_str());
    return;
}

void FSM::updateValuesToUi(ProfileName &curr) {
    ProfileValues values{};
    if (!settings_.readProfileValues(curr, values)) settings_.writeProfileValues(curr, values);

    ui_.setSessionMins(values.sessionMins);
    ui_.setRadarRangeCm(values.radarRangeCm);
    ui_.setAudioVolPct(values.audioVolPct);
    ui_.setLedBrightPct(values.ledBrightPct);

    SS_LOGI("UI updated for profile \"%s\"", curr.c_str());
}

} // namespace esphome::smart_signage::ctrl
