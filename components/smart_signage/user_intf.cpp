#include "user_intf.h"
#include "ctrl/ctrl_const.h"
#include "radar/radar_const.h"
#include "led/led_const.h"
#include "audio/audio_const.h"
#include "log.h"

#include <etl/to_string.h>

namespace esphome::smart_signage {

// ── static key definitions ─────────────────────────────────────────────────
const Key UserIntf::kCurrProfileKey  = "currProfile";
const Key UserIntf::kKnobFnKey       = "knobFn";
const Key UserIntf::kSessionMinsKey  = "sessionMins";
const Key UserIntf::kRadarRangeCmKey = "radarRangeCm";
const Key UserIntf::kAudioVolPctKey  = "audioVolPct";
const Key UserIntf::kLedBrightPctKey = "ledBrightPct";

// ── ctor ───────────────────────────────────────────────────────────────────
UserIntf::UserIntf(ConfigManager &cfg, const UiHandles &ui, const char *configJson)
    : cfg_(cfg), ui_(ui), configJsonstr_(configJson) {}

// ── public setup() — orchestrates the five private steps ───────────────────
bool UserIntf::setup() {
    if (!parseConfig()) return false;
    buildProfileOptions();
    buildKnobFnOptions();
    loadOrInitCurrentProfile();
    registerCallbacks();

    ui_.currProfile->publish_state(profile_options_[getCurrProfileIndex()]);
    return true;
}

// ───────────────────────────────────────────────────────────────────────────
//  STEP 1  : parse / validate JSON
// ───────────────────────────────────────────────────────────────────────────
bool UserIntf::parseConfig() {
    DeserializationError err = deserializeJson(configJsondoc_, configJsonstr_);
    if (err) {
        SS_LOGE("Config Json parse failed: %s", err.c_str());
        return false;
    }
    JsonArray profiles = configJsondoc_["profiles"].as<JsonArray>();
    if (profiles.isNull() || profiles.size() == 0) {
        SS_LOGE("No profiles found in JSON!");
        return false;
    }
    return true;
}

// ───────────────────────────────────────────────────────────────────────────
//  STEP 2  : build currProfile options
// ───────────────────────────────────────────────────────────────────────────
void UserIntf::buildProfileOptions() {
    JsonArray profiles = configJsondoc_["profiles"].as<JsonArray>();
    profile_options_.clear();
    for (JsonObject p : profiles) {
        const char *name = p["name"] | "<no name>";
        profile_options_.emplace_back(name);
    }
    ui_.currProfile->traits.set_options(profile_options_);
}

// ───────────────────────────────────────────────────────────────────────────
//  STEP 3  : build knob-function select options
// ───────────────────────────────────────────────────────────────────────────
void UserIntf::buildKnobFnOptions() {
    knobfn_options_ = {"Disabled"};
    if (ui_.sessionMins) knobfn_options_.emplace_back(ui_.sessionMins->get_name());
    if (ui_.radarRangeCm) knobfn_options_.emplace_back(ui_.radarRangeCm->get_name());
    if (ui_.audioVolPct) knobfn_options_.emplace_back(ui_.audioVolPct->get_name());
    if (ui_.ledBrightPct) knobfn_options_.emplace_back(ui_.ledBrightPct->get_name());
    ui_.knobFn->traits.set_options(knobfn_options_);
}

// ───────────────────────────────────────────────────────────────────────────
//  STEP 4  : load / initialise current profile idx in NVS
// ───────────────────────────────────────────────────────────────────────────
void UserIntf::loadOrInitCurrentProfile() {
    uint32_t idx = kDefaultProfileIdx;
    bool     ok  = cfg_.getValue(kCurrProfileKey, idx);
    if (!ok || idx >= profile_options_.size()) {
        cfg_.eraseAll();
        cfg_.setValue(kCurrProfileKey, kDefaultProfileIdx);
    }
}

// ───────────────────────────────────────────────────────────────────────────
//  STEP 5  : register all UI callbacks
// ───────────────────────────────────────────────────────────────────────────
void UserIntf::registerCallbacks() {
    using std::placeholders::_1;
    using std::placeholders::_2;

    ui_.currProfile->add_on_state_callback(
        std::bind(&UserIntf::onCurrProfileUpdated, this, _1, _2));
    ui_.knobFn->add_on_state_callback(std::bind(&UserIntf::onKnobFnUpdated, this, _1, _2));
    ui_.sessionMins->add_on_state_callback(std::bind(&UserIntf::onSessionMinsUpdated, this, _1));
    ui_.radarRangeCm->add_on_state_callback(std::bind(&UserIntf::onRadarRangeCmUpdated, this, _1));
    ui_.audioVolPct->add_on_state_callback(std::bind(&UserIntf::onAudioVolPctUpdated, this, _1));
    ui_.ledBrightPct->add_on_state_callback(std::bind(&UserIntf::onLedBrightPctUpdated, this, _1));
    ui_.startButton->add_on_press_callback(std::bind(&UserIntf::onStartButtonPressed, this));
}

// ───────────────────────────────────────────────────────────────────────────
//  UI callbacks
// ───────────────────────────────────────────────────────────────────────────
void UserIntf::onKnobFnUpdated(const std::string &, uint32_t idx) {
    SS_LOGI("onKnobFnUpdated");
    setNumber(kKnobFnKey, idx);
}

void UserIntf::onSessionMinsUpdated(float v) {
    SS_LOGI("onSessionMinsUpdated");
    setNumber(kSessionMinsKey, v);
}

void UserIntf::onRadarRangeCmUpdated(float v) {
    SS_LOGI("onRadarRangeCmUpdated");
    setNumber(kRadarRangeCmKey, v);
}

void UserIntf::onAudioVolPctUpdated(float v) {
    SS_LOGI("onAudioVolPctUpdated");
    setNumber(kAudioVolPctKey, v);
}

void UserIntf::onLedBrightPctUpdated(float v) {
    SS_LOGI("onLedBrightPctUpdated");
    setNumber(kLedBrightPctKey, v);
}

void UserIntf::onStartButtonPressed() { SS_LOGI("startButton pressed"); }

void UserIntf::onCurrProfileUpdated(const std::string &, uint32_t idx) {
    cfg_.setValue(kCurrProfileKey, idx);

    ui_.sessionMins->publish_state(getNumber(kSessionMinsKey, ctrl::kDefaultSessionMins));
    ui_.radarRangeCm->publish_state(getNumber(kRadarRangeCmKey, radar::kDefaultRangeCm));
    ui_.audioVolPct->publish_state(getNumber(kAudioVolPctKey, audio::kDefaultVolPct));
    ui_.ledBrightPct->publish_state(getNumber(kLedBrightPctKey, led::kDefaultBrightPct));
}

// ───────────────────────────────────────────────────────────────────────────
//  Storage helpers
// ───────────────────────────────────────────────────────────────────────────
uint32_t UserIntf::getNumber(Key key, uint32_t defaultValue) {
    uint32_t idx = getCurrProfileIndex();
    etl::to_string(idx, key, etl::format_spec(), true); // append profile index

    uint32_t v = defaultValue;
    cfg_.getValue(key, v, defaultValue);
    SS_LOGI("get %s → %u (profile %u)", key.c_str(), v, idx);
    return v;
}

void UserIntf::setNumber(Key key, float v) {
    uint32_t idx = getCurrProfileIndex();
    etl::to_string(idx, key, etl::format_spec(), true); // append profile index

    cfg_.setValue(key, static_cast<uint32_t>(v));
    SS_LOGI("set %s → %.1f (profile %u)", key.c_str(), v, idx);
}

uint32_t UserIntf::getCurrProfileIndex() const {
    uint32_t idx = 0;
    cfg_.getValue(kCurrProfileKey, idx);
    return idx;
}
} // namespace esphome::smart_signage
