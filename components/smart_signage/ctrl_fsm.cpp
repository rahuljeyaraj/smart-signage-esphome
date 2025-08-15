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
void FSM::boot() {
    SS_LOGI("Action: boot");

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

    vTaskDelay(2000 / portTICK_PERIOD_MS);

    SS_LOGI("broadcast CmdSetup");
    readyBits_.reset();
    radarQ_.post(radar::CmdSetup{});
    imuQ_.post(imu::CmdSetup{});
    ledQ_.post(led::CmdSetup{});
    audioQ_.post(audio::CmdSetup{});
}

void FSM::start() {
    driveOutput(profile::EventId::Start);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    SS_LOGI("Action: start -> broadcast CmdStart");
    radarQ_.post(radar::CmdStart{});
    imuQ_.post(imu::CmdStart{});
}

void FSM::stop() {
    SS_LOGI("Action: stop -> broadcast CmdStop");
    radarQ_.post(radar::CmdStop{});
    imuQ_.post(imu::CmdStop{});
    driveOutput(profile::EventId::Stop);
}

void FSM::teardown() {
    SS_LOGI("Action: teardown -> broadcast CmdTeardown");
    radarQ_.post(radar::CmdTeardown{});
    imuQ_.post(imu::CmdTeardown{});
    ledQ_.post(led::CmdTeardown{});
    audioQ_.post(audio::CmdTeardown{});
}

void FSM::onEvtRadarDistance(const EvtRadarDistance &e) {
    SS_LOGI("Data: distance=%u cm, timestamp=%u",
        static_cast<unsigned>(e.distanceCm),
        static_cast<unsigned>(e.timestampTicks));
    driveOutputForRadarDistance(e);
}
void FSM::onEvtRadarClear() {
    SS_LOGI("Event: Radar reports clear");
    // driveOutput(profile::EventId::Clear, true, false);
    audioQ_.post(audio::CmdStop{});
    ledQ_.post(led::CmdOff{});
}
void FSM::onEvtRadarDetected() {
    SS_LOGI("Event: Radar reports detected");
    driveOutput(profile::EventId::Detected, true, false);
}
void FSM::onEvtImuFell() {
    SS_LOGI("Event: IMU reports a fall detected!");
    driveOutput(profile::EventId::Fell);
}
void FSM::onEvtImuRose() {
    SS_LOGI("Event: IMU reports device has been restored (rose)");
    driveOutput(profile::EventId::Rose);
}

void FSM::onSetupTimeout() {
    SS_LOGI("Timeout: Setup phase timed out! Transitioning to Error state");
}

void FSM::onSessionEnd() {
    SS_LOGI("Timeout: Active phase timed out - stopping & tearing down all interfaces");
    stop();
    teardown();
}

void FSM::onUiProfileUpdate(const EvtUiProfileUpdate &e) {
    stop();
    ProfileName   curr = e.profileName;
    ProfileValues values{};
    settings_.writeCurrentProfile(curr);
    updateValuesToUi(curr);
    updateValuesToIntf(curr);
    enterReady();
}
void FSM::onUiSessionMinsUpdate(const EvtUiSessionMinsUpdate &e) {
    ProfileName   curr;
    ProfileValues values;
    settings_.readCurrentProfile(curr);
    settings_.readProfileValues(curr, values);
    values.sessionMins = e.mins;
    settings_.writeProfileValues(curr, values);
    sessionMins_ = e.mins;
}
void FSM::onUiRangeCmUpdate(const EvtUiRangeCmUpdate &e) {
    ProfileName   curr;
    ProfileValues values;
    settings_.readCurrentProfile(curr);
    settings_.readProfileValues(curr, values);
    values.radarRangeCm = e.cm;
    settings_.writeProfileValues(curr, values);
    ui_.setRadarRangeCm(values.radarRangeCm);
}
void FSM::onUiAudioVolUpdate(const EvtUiAudioVolUpdate &e) {
    ProfileName   curr;
    ProfileValues values;
    settings_.readCurrentProfile(curr);
    settings_.readProfileValues(curr, values);
    values.audioVolPct = e.pct;
    settings_.writeProfileValues(curr, values);
    ui_.setAudioVolPct(values.audioVolPct);
}
void FSM::onUiLedBrightUpdate(const EvtUiLedBrightUpdate &e) {
    ProfileName   curr;
    ProfileValues values;
    settings_.readCurrentProfile(curr);
    settings_.readProfileValues(curr, values);
    values.ledBrightPct = e.pct;
    settings_.writeProfileValues(curr, values);
    ui_.setLedBrightPct(values.ledBrightPct);
}

void FSM::enterReady() {
    SS_LOGI("Entered Ready state");
    // radarQ_.post(radar::SetRangeCm());
    driveOutput(profile::EventId::Ready);
}

void FSM::onError() {
    SS_LOGE("Entered Error state!");
    driveOutput(profile::EventId::Error);
}

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

void FSM::updateValuesToIntf(ProfileName &curr) {
    ProfileValues values{};
    if (!settings_.readProfileValues(curr, values)) settings_.writeProfileValues(curr, values);

    sessionMins_ = values.sessionMins;
    radarQ_.post(radar::SetRangeCm(values.radarRangeCm));
    audioQ_.post(audio::SetVolume(values.audioVolPct));
    ledQ_.post(led::SetBrightness(values.ledBrightPct));

    SS_LOGI("UI updated for profile \"%s\"", curr.c_str());
}

void FSM::driveOutput(profile::EventId ev, bool driveAudio, bool driveLed) {
    ProfileName curr{};
    if (!settings_.readCurrentProfile(curr)) {
        SS_LOGW("readCurrentProfile failed");
        return;
    }

    // // ── Audio ────────────────────────────────────────────────────────────────
    if (driveAudio) {
        audio::AudioPlaySpec audioPlaySpec;
        if (catalog_.getAudioPlaySpec(curr, ev, audioPlaySpec)) {
            SS_LOGW("play audio");
            audioQ_.post(audio::CmdPlay(audioPlaySpec));
        }
    }

    // ── LED ─────────────────────────────────────────────────────────────────
    if (driveLed) {
        led::LedPatternSpec ledPatternSpec{};
        if (catalog_.getLedPatternSpec(curr, ev, ledPatternSpec)) {
            SS_LOGW("play led");
            uint16_t t = static_cast<uint16_t>(ledPatternSpec.periodMs / 2u);
            uint16_t n = ledPatternSpec.cnt;

            switch (ledPatternSpec.pattern) {
            case led::LedPattern::Blink:
                // immediate high, hold t; immediate low, hold t
                ledQ_.post(led::CmdBreathe{0, t, 0, t, n});
                break;

            case led::LedPattern::Twinkle:
                // ramp up t, no high hold; ramp down t, no low hold
                ledQ_.post(led::CmdBreathe{t, 0, t, 0, n});
                break;

            default: SS_LOGW("unknown led pattern"); break;
            }
        }
    }
}

void FSM::driveOutputForRadarDistance(const EvtRadarDistance &e) {
    ProfileName   curr{};
    ProfileValues values{};
    if (!settings_.readCurrentProfile(curr)) {
        SS_LOGW("readCurrentProfile failed");
        return;
    }
    if (!settings_.readProfileValues(curr, values)) {
        SS_LOGW("readProfileValues failed");
        return;
    }

    led::LedPatternSpec max{};
    led::LedPatternSpec min{};
    if (!catalog_.getLedPatternSpec(curr, profile::EventId::DetectedDistanceMax, max)) { return; }
    if (!catalog_.getLedPatternSpec(curr, profile::EventId::DetectedDistanceMin, min)) { return; }

    const uint16_t range = values.radarRangeCm;
    const uint16_t dist  = e.distanceCm;
    // const uint16_t period = mapLinearClampedU16(dist, range, max.periodMs, 0u, min.periodMs);

    // uint16_t period = mapRange(dist, 0, 600, 2000, 200);
    uint16_t period = dist * 3;
    // SS_LOGD("radar dist=%u cm -> period=%u ms (min=%u max=%u range=%u)",
    //     dist,
    //     period,
    //     min.periodMs,
    //     max.periodMs,
    //     range);

    SS_LOGD("radar period=%u", period);

    uint16_t t = static_cast<uint16_t>(period / 2u);
    uint16_t n = 0;
    ledQ_.post(led::CmdBreathe{t, 0, t, 0, n});
}
uint16_t FSM::mapRange(
    uint16_t value, uint16_t inMin, uint16_t inMax, uint16_t outMin, uint16_t outMax) {
    // Normalize the input value to a 0-1 range (using float for intermediate calculations to avoid
    // truncation)
    float normalizedValue = static_cast<float>(value - inMin) / (inMax - inMin);

    // Scale and shift the normalized value to the output range (using float and then casting back
    // to uint16_t)
    return static_cast<uint16_t>(outMin + normalizedValue * (outMax - outMin));
}
// uint16_t FSM::mapLinearClampedU16(uint16_t x, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
// {
//     // clamp x to [min(x0,x1), max(x0,x1)]
//     uint16_t lo = (x0 < x1) ? x0 : x1;
//     uint16_t hi = (x0 < x1) ? x1 : x0;
//     uint16_t xc = x;
//     if (xc < lo) xc = lo;
//     if (xc > hi) xc = hi;

//     int32_t dx = (int32_t) x1 - (int32_t) x0; // can be negative
//     if (dx == 0) return y0;                   // degenerate, pick y0
//     int32_t dy  = (int32_t) y1 - (int32_t) y0;
//     int32_t num = (int32_t) (xc - x0) * dy; // signed
//     int32_t den = dx;

//     // round to nearest (works for ±den)
//     int32_t adj = ((den > 0 ? den : -den) >> 1);
//     int32_t y   = (int32_t) y0 + (num + (num >= 0 ? adj : -adj)) / den;

//     if (y < 0) y = 0;
//     if (y > 0xFFFF) y = 0xFFFF;
//     return (uint16_t) y;
// }

} // namespace esphome::smart_signage::ctrl
