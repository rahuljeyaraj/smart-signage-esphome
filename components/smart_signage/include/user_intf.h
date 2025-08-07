#pragma once

#include <vector>
#include <string>
#include <ArduinoJson.h>
#include <etl/string.h>
#include "config/config_manager.h"
#include "esphome/components/select/select.h"
#include "esphome/components/number/number.h"
#include "esphome/components/button/button.h"

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
    UserIntf(ConfigManager &cfg, const UiHandles &ui, const char *configJson);

    bool setup(); // call from your component’s setup()

  private:
    // ── one-time setup helpers ─────────────────────────────────────────────
    bool parseConfig();              // step 1: load & validate JSON
    void buildProfileOptions();      // step 2: fill currProfile options
    void buildKnobFnOptions();       // step 3: fill knobFn options
    void loadOrInitCurrentProfile(); // step 4: ensure valid profile idx
    void registerCallbacks();        // step 5: hook UI events

    // ── per-event callbacks ────────────────────────────────────────────────
    void onKnobFnUpdated(const std::string &v, uint32_t idx);
    void onSessionMinsUpdated(float v);
    void onRadarRangeCmUpdated(float v);
    void onAudioVolPctUpdated(float v);
    void onLedBrightPctUpdated(float v);
    void onStartButtonPressed();
    void onCurrProfileUpdated(const std::string &unused, uint32_t profileIdx);

    // ── small utilities ────────────────────────────────────────────────────
    uint32_t getNumber(Key baseKey, uint32_t defaultValue); // read NVS
    void     setNumber(Key baseKey, float v);               // write NVS
    uint32_t getCurrProfileIndex() const;

    static Key makeProfileKey(const Key &base, uint32_t idx);

    // ── data members ───────────────────────────────────────────────────────
    ConfigManager &cfg_;
    UiHandles      ui_;
    const char    *configJsonstr_;
    JsonDocument   configJsondoc_;

    std::vector<std::string> profile_options_; // cached for publish_state()
    std::vector<std::string> knobfn_options_;

    // storage-key literals
    static const Key kCurrProfileKey;
    static const Key kKnobFnKey;
    static const Key kSessionMinsKey;
    static const Key kRadarRangeCmKey;
    static const Key kAudioVolPctKey;
    static const Key kLedBrightPctKey;

    static constexpr char     kNamespace[]       = "SmartSignage";
    static constexpr char     TAG[]              = "UserIntf";
    static constexpr uint32_t kDefaultProfileIdx = 0;
    static constexpr uint32_t kDefaultKnobFnIdx  = 0;
};

} // namespace esphome::smart_signage
