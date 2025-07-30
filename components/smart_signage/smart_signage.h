#pragma once

#include "esphome.h"
#include "logger.h"
#include <freertos/message_buffer.h>

namespace esphome
{
  namespace smart_signage
  {

    // Log tag
    static constexpr char TAG[] = "ss";

    // Stub FSM declarations (one per task)
    struct RadarFsm
    {
      void process();
    };
    struct ImuFsm
    {
      void process();
    };
    struct ControllerFsm
    {
      void process();
    };
    struct LedFsm
    {
      void process();
    };
    struct SpeakerFsm
    {
      void process();
    };

    struct Settings
    {
      float radius_m = 2.0f;
      uint32_t duration_s = 5;
      uint8_t volume = 80;
      uint8_t brightness = 50;
    };

    class SmartSignage : public Component
    {
    public:
      // these setters are invoked by the Number callbacks
      void set_radius(float v)
      {
        settings_.radius_m = v;
        LOGI(TAG, "radius %.2f", v);
      }
      void set_duration(float v)
      {
        settings_.duration_s = static_cast<uint32_t>(v);
        LOGI(TAG, "duration %u", settings_.duration_s);
      }
      void set_volume(float v)
      {
        settings_.volume = static_cast<uint8_t>(v);
        LOGI(TAG, "volume %u", settings_.volume);
      }
      void set_brightness(float v)
      {
        settings_.brightness = static_cast<uint8_t>(v);
        LOGI(TAG, "brightness %u", settings_.brightness);
      }

      void on_start_button();
      void setup() override;
      void loop() override;
      void dump_config() override;

    private:
      Settings settings_;

      // FSM instances
      RadarFsm radar_fsm_;
      ImuFsm imu_fsm_;
      ControllerFsm controller_fsm_;
      LedFsm led_fsm_;
      SpeakerFsm speaker_fsm_;

      // Task entry points
      static void radar_task_entry(void *pv);

      static void controller_task_entry(void *pv);
      static void led_task_entry(void *pv);
      static void speaker_task_entry(void *pv);

      QueueHandle_t controller_queue_{};
      QueueHandle_t led_queue_{};
      QueueHandle_t speaker_queue_{};
    };

  } // namespace smart_signage
} // namespace esphome
