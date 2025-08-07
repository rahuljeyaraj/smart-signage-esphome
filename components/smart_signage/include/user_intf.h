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
#include <etl/string.h>    // For etl::string
#include <etl/to_string.h> // For etl::to_string

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

        ui_.currProfile->publish_state(profile_options[currProfileIdx]);

        // publishSelect(kKnobFnKey, ui_.knobFn, kDefaultKnobFnIdx);
        publishNumber(kSessionMinsKey, ui_.sessionMins, ctrl::kDefaultSessionMins);
        publishNumber(kRadarRangeCmKey, ui_.sessionMins, radar::kDefaultRangeCm);
        publishNumber(kAudioVolPctKey, ui_.sessionMins, audio::kDefaultVolPct);
        publishNumber(kLedBrightPctKey, ui_.sessionMins, led::kDefaultBrightPct);

        registerCallbacks();
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

    // Each callback updates the value for the current profile index in NVS
    inline void onCurrProfileUpdated(const std::string &v, uint32_t idx) {
        set(kCurrProfileKey, idx);
    }
    inline void onKnobFnUpdated(const std::string &v, uint32_t idx) { set(kKnobFnKey, idx); }
    inline void onSessionMinsUpdated(float v) { set(kSessionMinsKey, v); }
    inline void onRadarRangeCmUpdated(float v) { set(kRadarRangeCmKey, v); }
    inline void onAudioVolPctUpdated(float v) { set(kAudioVolPctKey, v); }
    inline void onLedBrightPctUpdated(float v) { set(kLedBrightPctKey, v); }
    inline void onStartButtonPressed() { LOGI("startButton pressed"); }

    inline void publishNumber(Key key, number::Number *ui_field, uint32_t defaultValue) {
        uint32_t value;
        get(key, value, defaultValue);
        ui_field->publish_state(static_cast<float>(value));
        LOGI("publish %s → %u", key.c_str(), value);
    }

    // inline void publishSelect(Key key, select::Select *ui_field, uint32_t defaultValue) {
    //     uint32_t value;
    //     get(key, value, defaultValue);
    //     ui_field.publish_state(profile_options[idx]);
    //     LOGI("publish select → %s (profile %u)", profile_options[idx].c_str(), idx);
    // }

    // inline void set(Key key, float v) {
    //     uint32_t idx = getCurrProfileIndex();
    //     key += etl::to_string(idx);
    //     cfg_.setValue(key, static_cast<uint32_t>(v));
    //     LOGI("set %s → %.1f (profile %u)", key.c_str(), v, idx);
    // }

    // inline void get(Key key, uint32_t &value, uint32_t defaultValue) {
    //     uint32_t idx = getCurrProfileIndex();
    //     key += etl::to_string(idx);
    //     cfg_.getValue(key, value, defaultValue);
    //     LOGI("get %s → %u (profile %u)", key.c_str(), value, idx);
    // }

    inline void set(Key key, float v) {
        uint32_t idx = getCurrProfileIndex();
        etl::to_string(idx, key);
        cfg_.setValue(key, static_cast<uint32_t>(v));
        LOGI("set %s → %.1f (profile %u)", key.c_str(), v, idx);
    }

    inline void get(Key key, uint32_t &value, uint32_t defaultValue) {
        uint32_t idx = getCurrProfileIndex();
        etl::to_string(idx, key); // appends idx to key
        cfg_.getValue(key, value, defaultValue);
        LOGI("get %s → %u (profile %u)", key.c_str(), value, idx);
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
