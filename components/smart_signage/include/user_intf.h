#pragma once

#include "config/config_manager.h"
#include "esphome/components/select/select.h"
#include "esphome/components/number/number.h"
#include "esphome/components/button/button.h"
#include "esp_log.h"
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
    UserIntf(ConfigManager &cfg, const UiHandles &ui) : cfg_(cfg), ui_(ui) { registerCallbacks(); }

  private:
    void registerCallbacks() {
        ui_.currProfile->add_on_state_callback(
            [this](const std::string &v) { this->onCurrProfileUpdate(v); });
        ui_.sessionMins->add_on_state_callback([this](float v) { this->onSessionMinsUpdate(v); });
        ui_.radarRangeCm->add_on_state_callback([this](float v) { this->onRadarRangeCmUpdate(v); });
        ui_.audioVolPct->add_on_state_callback([this](float v) { this->onAudioVolPctUpdate(v); });
        ui_.ledBrightPct->add_on_state_callback([this](float v) { this->onLedBrightPctUpdate(v); });
        ui_.startButton->set_on_press_callback([this]() { this->onStartButtonPressed(); });
        ui_.knobFn->add_on_state_callback(
            [this](const std::string &v) { this->onKnobFnUpdate(v); });
    }

    // — callback implementations —
    inline void onCurrProfileUpdate(const std::string &newProfile) {
        cfg_.setString(ns_, kCurrProfileKey, newProfile);
        ESP_LOGI("userIntf", "currProfile → %s", newProfile.c_str());
    }
    inline void onSessionMinsUpdate(float newMins) {
        cfg_.setValue(ns_, kSessionMinsKey, static_cast<uint32_t>(newMins));
        ESP_LOGI("userIntf", "sessionMins → %.1f", newMins);
    }
    inline void onRadarRangeCmUpdate(float newRange) {
        cfg_.setValue(ns_, kRadarRangeCmKey, static_cast<uint32_t>(newRange));
        ESP_LOGI("userIntf", "radarRangeCm → %.1f", newRange);
    }
    inline void onAudioVolPctUpdate(float newVol) {
        cfg_.setValue(ns_, kAudioVolPctKey, static_cast<uint32_t>(newVol));
        ESP_LOGI("userIntf", "audioVolPct → %.1f", newVol);
    }
    inline void onLedBrightPctUpdate(float newPct) {
        cfg_.setValue(ns_, kLedBrightPctKey, static_cast<uint32_t>(newPct));
        ESP_LOGI("userIntf", "ledBrightPct → %.1f", newPct);
    }
    inline void onStartButtonPressed() {
        ESP_LOGI("userIntf", "startButton pressed");
        // TODO: trigger main action
    }
    inline void onKnobFnUpdate(const std::string &newFn) {
        cfg_.setString(ns_, kKnobFnKey, newFn);
        ESP_LOGI("userIntf", "knobFn → %s", newFn.c_str());
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
};

} // namespace esphome::smart_signage
