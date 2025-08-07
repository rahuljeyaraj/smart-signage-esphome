#pragma once

#include "config/config_manager.h"
#include "esphome/components/select/select.h"
#include "esphome/components/number/number.h"
#include "esphome/components/button/button.h"
#include "log.h"
#include <string>
#include <ArduinoJson.h>

namespace esphome::smart_signage {

/** Bundle all UI- and ADC-handles into one POD */
struct UiHandles {
    select::Select *currProfile;
    number::Number *sessionMins;
    number::Number *radarRangeCm;
    number::Number *audioVolPct;
    number::Number *ledBrightPct;
    button::Button *startButton;
    select::Select *knobFn;
    // adc_oneshot_unit_handle_t *knobHandle;
};

class UserIntf {
  public:
    UserIntf(ConfigManager &cfg, const UiHandles &ui, const char *configJson) : cfg_(cfg), ui_(ui) {
        DeserializationError err = deserializeJson(configJsondoc_, configJson);
        if (err) {
            LOGE("Config Json parse failed: %s", err.c_str());
            // TODO: handle error
        }
    }

  public:
    void setup() {
        JsonArray profiles = configJsondoc_["profiles"].as<JsonArray>();

        if (profiles.isNull()) {
            LOGE("No profiles found in JSON!");
            return;
        }

        LOGI("List of Profiles:");
        std::vector<std::string> options;
        for (JsonObject profile : profiles) {
            const char *profileName = profile["name"] | "<no name>";
            LOGI("Name: %s", profileName);
            options.emplace_back(profileName);
        }

        // Set the options in the select component
        ui_.currProfile->traits.set_options(options);
    }
    // JsonArray profiles = configJsondoc_["profiles"].as<JsonArray>();

    // LOGI("List of Profiles:");
    // for (JsonObject profile : profiles) {
    //     const char *profileId   = profile["id"];
    //     const char *profileName = profile["name"];
    //     LOGI("ID: %s", profileId);
    //     LOGI("Name: %s", profileName);
    //     ui_.currProfile->traits.set_options({"__placeholder__"});
    // }

    void registerCallbacks() {
        using std::placeholders::_1;
        using std::placeholders::_2;

        ui_.currProfile->add_on_state_callback(
            std::bind(&UserIntf::onCurrProfileChanged, this, _1, _2));
        ui_.knobFn->add_on_state_callback(std::bind(&UserIntf::onKnobFnChanged, this, _1, _2));
        ui_.sessionMins->add_on_state_callback(
            std::bind(&UserIntf::onSessionMinsChanged, this, _1));
        ui_.radarRangeCm->add_on_state_callback(
            std::bind(&UserIntf::onRadarRangeCmChanged, this, _1));
        ui_.audioVolPct->add_on_state_callback(
            std::bind(&UserIntf::onAudioVolPctChanged, this, _1));
        ui_.ledBrightPct->add_on_state_callback(
            std::bind(&UserIntf::onLedBrightPctChanged, this, _1));
        ui_.startButton->add_on_press_callback(std::bind(&UserIntf::onStartButtonPressed, this));
    }

  private:
    inline void onCurrProfileChanged(const std::string &v, uint32_t idx) {
        cfg_.setString(ns_, kCurrProfileKey, v);
        LOGI("currProfile → %s (idx %u)", v.c_str(), idx);
    }
    inline void onKnobFnChanged(const std::string &v, uint32_t idx) {
        cfg_.setString(ns_, kKnobFnKey, v);
        LOGI("knobFn → %s (idx %u)", v.c_str(), idx);
    }
    inline void onSessionMinsChanged(float v) {
        cfg_.setValue(ns_, kSessionMinsKey, static_cast<uint32_t>(v));
        LOGI("sessionMins → %.1f", v);
    }
    inline void onRadarRangeCmChanged(float v) {
        cfg_.setValue(ns_, kRadarRangeCmKey, static_cast<uint32_t>(v));
        LOGI("radarRangeCm → %.1f", v);
    }
    inline void onAudioVolPctChanged(float v) {
        cfg_.setValue(ns_, kAudioVolPctKey, static_cast<uint32_t>(v));
        LOGI("audioVolPct → %.1f", v);
    }
    inline void onLedBrightPctChanged(float v) {
        cfg_.setValue(ns_, kLedBrightPctKey, static_cast<uint32_t>(v));
        LOGI("ledBrightPct → %.1f", v);
    }
    inline void onStartButtonPressed() {
        LOGI("startButton pressed");
        // TODO: your start action
    }

    // — storage keys —
    static constexpr char kCurrProfileKey[]  = "currProfile";
    static constexpr char kKnobFnKey[]       = "knobFn";
    static constexpr char kSessionMinsKey[]  = "sessionMins";
    static constexpr char kRadarRangeCmKey[] = "radarRangeCm";
    static constexpr char kAudioVolPctKey[]  = "audioVolPct";
    static constexpr char kLedBrightPctKey[] = "ledBrightPct";

    ConfigManager           &cfg_;
    UiHandles                ui_;
    ConfigManager::Namespace ns_{"ss"};
    JsonDocument             configJsondoc_;

    static constexpr char TAG[] = "UserIntf";
};

} // namespace esphome::smart_signage
