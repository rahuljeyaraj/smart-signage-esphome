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

    class SmartSignage : public Component
    {
    public:
      void setup() override;
      void loop() override;
      void dump_config() override;

    private:
      // One FreeRTOS message buffer per task
      MessageBufferHandle_t radar_buffer_;
      MessageBufferHandle_t imu_buffer_;
      MessageBufferHandle_t controller_buffer_;
      MessageBufferHandle_t led_buffer_;
      MessageBufferHandle_t speaker_buffer_;

      // FSM instances
      RadarFsm radar_fsm_;
      ImuFsm imu_fsm_;
      ControllerFsm controller_fsm_;
      LedFsm led_fsm_;
      SpeakerFsm speaker_fsm_;

      // Task entry points
      static void radar_task_entry(void *pv);
      static void imu_task_entry(void *pv);
      static void controller_task_entry(void *pv);
      static void led_task_entry(void *pv);
      static void speaker_task_entry(void *pv);
    };

  } // namespace smart_signage
} // namespace esphome
