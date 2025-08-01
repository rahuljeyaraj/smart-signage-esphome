#pragma once
#include "esphome.h"
#include "log.h"


namespace esphome::smart_signage {

class SmartSignage : public Component {
  public:
    void set_radius(float v) {
        settings_.radius_m = v;
        LOGI(TAG, "radius %.2f", v);
    }
    void set_duration(float v) {
        settings_.duration_s = v;
        LOGI(TAG, "duration %u", settings_.duration_s);
    }
    void set_volume(float v) {
        settings_.volume = v;
        LOGI(TAG, "volume %u", settings_.volume);
    }
    void set_brightness(float v) {
        settings_.brightness = v;
        LOGI(TAG, "brightness %u", settings_.brightness);
    }
    void on_start_button() {
        LOGI(TAG, "Start btn pressed"); /* later: g_fsm.receive(StartBtnMsg{}) */
    }

    // ── ESPHome lifecycle ────────────────────────────────
    void setup() override;
    void loop() override;
    void dump_config() override;

  private:
    static constexpr char TAG[] = "ss";

    struct Settings {
        float radius_m = 2.0f;
        uint32_t duration_s = 30;
        uint8_t volume = 80;
        uint8_t brightness = 50;
    } settings_;
};

} // namespace esphome::smart_signage
