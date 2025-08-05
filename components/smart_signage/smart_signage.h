#pragma once
#include "esphome.h"
#include <esphome/components/number/number.h>
#include <esphome/components/select/select.h>
#include <map>
#include <vector>

namespace esphome::smart_signage {

struct UserSettings {
    float    radius_m   = 2.0f; // metres
    uint32_t duration_s = 3600; // seconds
    uint8_t  volume     = 80;   // %
    uint8_t  brightness = 50;   // %
};

class SmartSignage : public Component {
  public:
    using PathList   = std::vector<std::string>;
    using EventMap   = std::map<std::string, PathList>;
    using ProfileMap = std::map<std::string, EventMap>;

    /*── build-time hooks ───────────────────────────────*/
    void add_event_map(const std::string &profile, const std::string &evt, const PathList &paths);

    /*── injected entity pointers (from Python) ─────────*/
    void set_radius_number(number::Number *n) { radius_num_ = n; }
    void set_duration_number(number::Number *n) { duration_num_ = n; }
    void set_volume_number(number::Number *n) { volume_num_ = n; }
    void set_brightness_number(number::Number *n) { brightness_num_ = n; }
    void set_profile_select(select::Select *s) { profile_sel_ = s; }

    /*── UI callbacks ───────────────────────────────────*/
    void set_profile(const std::string &p);
    void set_radius(float v);
    void set_duration(float v);
    void set_volume(float v);
    void set_brightness(float v);
    void on_start_button();

    /*── ESPHome lifecycle ──────────────────────────────*/
    void setup() override;
    void dump_config() override {}

  private:
    /*── helpers ────────────────────────────────────────*/
    bool load_from_nvs(const std::string &profile, UserSettings &out);
    void save_to_nvs(const std::string &profile, const UserSettings &in);
    void publish_to_dashboard(const UserSettings &s);

    /*── entity handles ────────────────────────────────*/
    number::Number *radius_num_{nullptr};
    number::Number *duration_num_{nullptr};
    number::Number *volume_num_{nullptr};
    number::Number *brightness_num_{nullptr};
    select::Select *profile_sel_{nullptr};

    /*── data ───────────────────────────────────────────*/
    ProfileMap                          profiles_;
    std::map<std::string, UserSettings> cached_;
    std::string                         current_profile_;
    UserSettings                        settings_;
};

} // namespace esphome::smart_signage
