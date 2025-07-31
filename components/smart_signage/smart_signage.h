#pragma once
#include "esphome.h"
#include "logger.h"

namespace esphome::smart_signage
{

    class SmartSignage : public Component
    {
    public:
        // ── YAML-bound setters ───────────────────────────────
        // void set_radius(float m);     // slider “radius”
        // void set_duration(float s);   // slider “duration”
        // void set_volume(float v);     // slider “volume”
        // void set_brightness(float b); // slider “brightness”
        // void on_start_button();       // binary_sensor / button

        void set_radius(float v)
        {
            settings_.radius_m = v;
            LOGI(TAG, "radius %.2f", v);
        }
        void set_duration(float v)
        {
            settings_.duration_s = v;
            LOGI(TAG, "duration %u", settings_.duration_s);
        }
        void set_volume(float v)
        {
            settings_.volume = v;
            LOGI(TAG, "volume %u", settings_.volume);
        }
        void set_brightness(float v)
        {
            settings_.brightness = v;
            LOGI(TAG, "brightness %u", settings_.brightness);
        }
        void on_start_button()
        {
            LOGI(TAG, "Start btn pressed"); /* later: g_fsm.receive(StartBtnMsg{}) */
        }

        // ── ESPHome lifecycle ────────────────────────────────
        void setup() override;
        void loop() override;
        void dump_config() override;

    private:
        static constexpr char TAG[] = "ss";

        struct Settings
        {
            float radius_m = 2.0f;
            uint32_t duration_s = 30;
            uint8_t volume = 80;
            uint8_t brightness = 50;
        } settings_;
    };

} // namespace esphome::smart_signage

// #pragma once

// #include "esphome.h"
// #include "logger.h"
// #include "message_protocol.h"
// #include "controller_fsm.h"
// #include <freertos/FreeRTOS.h>
// #include <freertos/queue.h>
// #include <memory>

// namespace esphome::smart_signage
// {
//     static constexpr char TAG[] = "ss";

//     struct Settings
//     {
//         float radius_m = 2.0f;
//         uint32_t duration_s = 5;
//         uint8_t volume = 80;
//         uint8_t brightness = 50;
//     };

//     class SmartSignage : public Component
//     {
//     public:
//         // these setters are invoked by the Number callbacks
//         void set_radius(float v)
//         {
//             settings_.radius_m = v;
//             LOGI(TAG, "radius %.2f", v);
//         }
//         void set_duration(float v)
//         {
//             settings_.duration_s = static_cast<uint32_t>(v);
//             LOGI(TAG, "duration %u", settings_.duration_s);
//         }
//         void set_volume(float v)
//         {
//             settings_.volume = static_cast<uint8_t>(v);
//             LOGI(TAG, "volume %u", settings_.volume);
//         }
//         void set_brightness(float v)
//         {
//             settings_.brightness = static_cast<uint8_t>(v);
//             LOGI(TAG, "brightness %u", settings_.brightness);
//         }
//         void on_start_button()
//         {
//             LOGI(TAG, "Start button pressed — radius=%.2f m, duration=%u s, vol=%u, bri=%u",
//                  settings_.radius_m, settings_.duration_s,
//                  settings_.volume, settings_.brightness);
//             // dispatch start event to FSM here...
//         }
//         void setup() override;
//         void loop() override;
//         void dump_config() override;

//     private:
//         Settings settings_;

//         QueueHandle_t controller_queue_{};
//         QueueHandle_t led_queue_{};
//         QueueHandle_t speaker_queue_{};

//         // Task entry points
//         static void radar_task_entry(void *pv);
//         static void led_task_entry(void *pv);
//         static void speaker_task_entry(void *pv);
//     };

// } // namespace esphome::smart_signage