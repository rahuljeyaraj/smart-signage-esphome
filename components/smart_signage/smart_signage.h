#pragma once
#include "esphome.h"
#include <map>
#include <vector>

namespace esphome::smart_signage {

class SmartSignage : public Component {
  public:
    using PathList   = std::vector<std::string>;        // 1‒3 items
    using EventMap   = std::map<std::string, PathList>; // key = event
    using ProfileMap = std::map<std::string, EventMap>; // key = profile

    // Called from code-gen (see __init__.py)
    void add_event_map(const std::string &profile, const std::string &evt, const PathList &paths) {
        profiles_[profile][evt] = paths;
    }

    // ───────── Dashboard callbacks ─────────
    void set_profile(const std::string &p);
    void set_radius(float v);
    void set_duration(float v);
    void set_volume(float v);
    void set_brightness(float v);
    void on_start_button();

    // ───────── ESPHome lifecycle ───────────
    void setup() override;
    void loop() override;
    void dump_config() override;

  private:
    ProfileMap  profiles_;
    std::string current_profile_;

    struct Settings {
        float    radius_m   = 2.0f;
        uint32_t duration_s = 30;
        uint8_t  volume     = 80;
        uint8_t  brightness = 50;
    } settings_;
};

} // namespace esphome::smart_signage

// #pragma once
// #include "esphome.h"
// #include "log.h"

// namespace esphome::smart_signage {

// class SmartSignage : public Component {
//   public:
//     number::Number *radius_number_{nullptr};

//     void set_profile(const std::string &p) {
//         // settings_.radius_m = v;
//         LOGI("profile %s", p.c_str());
//     }
//     void set_radius(float v) {
//         settings_.radius_m = v;
//         LOGI("radius %.2f", v);
//         number::Number *radius_number_{nullptr};
//     }
//     void set_duration(float v) {
//         settings_.duration_s = v;
//         LOGI("duration %u", settings_.duration_s);
//     }
//     void set_volume(float v) {
//         settings_.volume = v;
//         LOGI("volume %u", settings_.volume);
//     }
//     void set_brightness(float v) {
//         settings_.brightness = v;
//         LOGI("brightness %u", settings_.brightness);
//     }
//     void on_start_button() { LOGI("Start btn pressed"); /* later: g_fsm.receive(StartBtnMsg{}) */
//     }

//     // ── ESPHome lifecycle ────────────────────────────────
//     void setup() override;
//     void loop() override;
//     void dump_config() override;

//   private:
//     static constexpr char TAG[] = "ss";

//     struct Settings {
//         float    radius_m   = 2.0f;
//         uint32_t duration_s = 30;
//         uint8_t  volume     = 80;
//         uint8_t  brightness = 50;
//     } settings_;
// };

// } // namespace esphome::smart_signage
