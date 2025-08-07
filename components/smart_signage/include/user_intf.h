#pragma once

#include "config/config_manager.h"
#include "esphome/components/select/select.h"
#include "esphome/components/number/number.h"
#include "esphome/components/button/button.h"
#include "log.h"
#include <string>

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
    UserIntf(ConfigManager &cfg, const UiHandles &ui) : cfg_(cfg), ui_(ui) {}

  public:
    void registerCallbacks() {
        using std::placeholders::_1;
        using std::placeholders::_2;

        // select: value + index
        ui_.currProfile->add_on_state_callback(
            std::bind(&UserIntf::onCurrProfileChanged, this, _1, _2));
        ui_.knobFn->add_on_state_callback(std::bind(&UserIntf::onKnobFnChanged, this, _1, _2));

        // number: only value
        ui_.sessionMins->add_on_state_callback(
            std::bind(&UserIntf::onSessionMinsChanged, this, _1));
        ui_.radarRangeCm->add_on_state_callback(
            std::bind(&UserIntf::onRadarRangeCmChanged, this, _1));
        ui_.audioVolPct->add_on_state_callback(
            std::bind(&UserIntf::onAudioVolPctChanged, this, _1));
        ui_.ledBrightPct->add_on_state_callback(
            std::bind(&UserIntf::onLedBrightPctChanged, this, _1));

        // button
        ui_.startButton->add_on_press_callback(std::bind(&UserIntf::onStartButtonPressed, this));
    }

  private:
    // — select callbacks take (value, idx) and log + save —
    inline void onCurrProfileChanged(const std::string &v, uint32_t idx) {
        cfg_.setString(ns_, kCurrProfileKey, v);
        LOGI("currProfile → %s (idx %u)", v.c_str(), idx);
    }
    inline void onKnobFnChanged(const std::string &v, uint32_t idx) {
        cfg_.setString(ns_, kKnobFnKey, v);
        LOGI("knobFn → %s (idx %u)", v.c_str(), idx);
    }

    // — number callbacks take only (value) —
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

    // — button callback —
    inline void onStartButtonPressed() {
        LOGI("startButton pressed");
        // TODO: your start action
    }

    // — storage keys —
    static constexpr char kCurrProfileKey[]  = "currProfile";
    static constexpr char kSessionMinsKey[]  = "sessionMins";
    static constexpr char kRadarRangeCmKey[] = "radarRangeCm";
    static constexpr char kAudioVolPctKey[]  = "audioVolPct";
    static constexpr char kLedBrightPctKey[] = "ledBrightPct";
    static constexpr char kKnobFnKey[]       = "knobFn";

    ConfigManager           &cfg_;
    UiHandles                ui_;
    ConfigManager::Namespace ns_{"ss"};

    static constexpr char TAG[] = "UserIntf";
};

} // namespace esphome::smart_signage
