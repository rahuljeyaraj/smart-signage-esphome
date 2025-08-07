#pragma once

#include "config/config_manager.h"
#include "esphome/components/select/select.h"
#include "esphome/components/number/number.h"
#include "esphome/components/button/button.h"
#include "log.h"
#include <string>
#include <ArduinoJson.h>
#include <vector>
#include "ctrl/ctrl_const.h"
#include "radar/radar_const.h"
#include "led/led_const.h"
#include "audio/audio_const.h"
#include <etl/string.h>
#include <etl/to_string.h>
#include <cstdio>

namespace esphome::smart_signage {

struct UiHandles {
    select::Select *currProfile;
    number::Number *sessionMins;
    number::Number *radarRangeCm;
    number::Number *audioVolPct;
    number::Number *ledBrightPct;
    button::Button *startButton;
    select::Select *knobFn;
};

class UserIntf {
  public:
    UserIntf(ConfigManager &cfg, const UiHandles &ui, const char *configJson)
        : cfg_(cfg), ui_(ui), configJsonstr_(configJson) {}

    bool setup() {

        // func 1
        DeserializationError err = deserializeJson(configJsondoc_, configJsonstr_);
        if (err) {
            LOGE("Config Json parse failed: %s", err.c_str());
            return false;
        }
        JsonArray profiles = configJsondoc_["profiles"].as<JsonArray>();
        if (profiles.isNull() || profiles.size() == 0) {
            LOGE("No profiles found in JSON!");
            return false;
        }

        // func 2
        // ----- Setup currProfile select options -----
        std::vector<std::string> profile_options;
        for (JsonObject profile : profiles) {
            const char *profileName = profile["name"] | "<no name>";
            profile_options.emplace_back(profileName);
        }
        ui_.currProfile->traits.set_options(profile_options);

        // func 3
        // ----- Setup knobFn options -----
        std::vector<std::string> knobFn_options{"Disabled"};
        if (ui_.sessionMins) knobFn_options.emplace_back(ui_.sessionMins->get_name());
        if (ui_.radarRangeCm) knobFn_options.emplace_back(ui_.radarRangeCm->get_name());
        if (ui_.audioVolPct) knobFn_options.emplace_back(ui_.audioVolPct->get_name());
        if (ui_.ledBrightPct) knobFn_options.emplace_back(ui_.ledBrightPct->get_name());
        ui_.knobFn->traits.set_options(knobFn_options);

        // func 4
        //  ----- Get currProfile index -----
        size_t   num_profiles     = profiles.size();
        uint32_t currProfileIdx   = kDefaultProfileIdx;
        bool     foundCurrProfile = cfg_.getValue(kCurrProfileKey, currProfileIdx);
        if (!foundCurrProfile || currProfileIdx >= num_profiles) {
            cfg_.eraseAll();
            cfg_.setValue(kCurrProfileKey, kDefaultProfileIdx);
        }

        registerCallbacks();

        ui_.currProfile->publish_state(profile_options[currProfileIdx]);
        return true;
    }

  private:
    void registerCallbacks() {
        using std::placeholders::_1;
        using std::placeholders::_2;

        ui_.currProfile->add_on_state_callback(
            std::bind(&UserIntf::onCurrProfileUpdated, this, _1, _2));
        ui_.knobFn->add_on_state_callback(std::bind(&UserIntf::onKnobFnUpdated, this, _1, _2));
        ui_.sessionMins->add_on_state_callback(
            std::bind(&UserIntf::onSessionMinsUpdated, this, _1));
        ui_.radarRangeCm->add_on_state_callback(
            std::bind(&UserIntf::onRadarRangeCmUpdated, this, _1));
        ui_.audioVolPct->add_on_state_callback(
            std::bind(&UserIntf::onAudioVolPctUpdated, this, _1));
        ui_.ledBrightPct->add_on_state_callback(
            std::bind(&UserIntf::onLedBrightPctUpdated, this, _1));
        ui_.startButton->add_on_press_callback(std::bind(&UserIntf::onStartButtonPressed, this));
    }

    void onKnobFnUpdated(const std::string &v, uint32_t idx) {
        LOGI("onKnobFnUpdated");
        set(kKnobFnKey, idx);
    }
    void onSessionMinsUpdated(float v) {
        LOGI("onSessionMinsUpdated");
        set(kSessionMinsKey, v);
    }
    void onRadarRangeCmUpdated(float v) {
        LOGI("onRadarRangeCmUpdated");
        set(kRadarRangeCmKey, v);
    }
    void onAudioVolPctUpdated(float v) {
        LOGI("onAudioVolPctUpdated");
        set(kAudioVolPctKey, v);
    }
    void onLedBrightPctUpdated(float v) {
        LOGI("onLedBrightPctUpdated");
        set(kLedBrightPctKey, v);
    }

    void onStartButtonPressed() { LOGI("startButton pressed"); }

    void onCurrProfileUpdated(const std::string & /*unused*/, uint32_t profileIdx) {
        cfg_.setValue(kCurrProfileKey, profileIdx);
        ui_.sessionMins->publish_state(get(kSessionMinsKey, ctrl::kDefaultSessionMins));
        ui_.radarRangeCm->publish_state(get(kRadarRangeCmKey, radar::kDefaultRangeCm));
        ui_.audioVolPct->publish_state(get(kAudioVolPctKey, audio::kDefaultVolPct));
        ui_.ledBrightPct->publish_state(get(kLedBrightPctKey, led::kDefaultBrightPct));
    }

    uint32_t get(Key key, uint32_t defaultValue) {
        uint32_t idx = getCurrProfileIndex();
        etl::to_string(idx, key, etl::format_spec(), true);
        uint32_t v = defaultValue;
        cfg_.getValue(key, v, defaultValue);
        LOGI("get %s → %u  (profile %u)", key.c_str(), v, idx);
        return v;
    }

    void set(Key key, float v) {
        uint32_t idx = getCurrProfileIndex();
        etl::to_string(idx, key, etl::format_spec(), true);
        cfg_.setValue(key, static_cast<uint32_t>(v));
        LOGI("set %s → %.1f (profile %u)", key.c_str(), v, idx);
    }

    uint32_t getCurrProfileIndex() const {
        uint32_t idx = 0;
        cfg_.getValue(kCurrProfileKey, idx);
        return idx;
    }

    ConfigManager &cfg_;
    UiHandles      ui_;
    const char    *configJsonstr_;
    JsonDocument   configJsondoc_;

    // — storage keys —
    const Key kCurrProfileKey  = "currProfile";
    const Key kKnobFnKey       = "knobFn";
    const Key kSessionMinsKey  = "sessionMins";
    const Key kRadarRangeCmKey = "radarRangeCm";
    const Key kAudioVolPctKey  = "audioVolPct";
    const Key kLedBrightPctKey = "ledBrightPct";

    static constexpr char kNamespace[] = "SmartSignage";
    static constexpr char TAG[]        = "UserIntf";

    static constexpr char kDefaultProfileIdx = 0;
    static constexpr char kDefaultKnobFnIdx  = 0;
};

} // namespace esphome::smart_signage
