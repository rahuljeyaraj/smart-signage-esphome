#include "smart_signage.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
// #include <boost/sml.hpp>

namespace esphome
{
    namespace smart_signage
    {

        // --- FSM stubs ---
        void RadarFsm::process() { LOGI(TAG, "RadarFsm tick"); }
        void ImuFsm::process() { LOGI(TAG, "ImuFsm tick"); }
        void ControllerFsm::process() { LOGI(TAG, "ControllerFsm tick"); }
        void LedFsm::process() { LOGI(TAG, "LedFsm tick"); }
        void SpeakerFsm::process() { LOGI(TAG, "SpeakerFsm tick"); }

        // --- Component methods ---
        void SmartSignage::setup()
        {
            // Create buffers (64 bytes each for now)
            radar_buffer_ = xMessageBufferCreate(64);
            imu_buffer_ = xMessageBufferCreate(64);
            controller_buffer_ = xMessageBufferCreate(64);
            led_buffer_ = xMessageBufferCreate(64);
            speaker_buffer_ = xMessageBufferCreate(64);

            // Spawn tasks
            xTaskCreate(radar_task_entry, "radar", 4096, this, 1, nullptr);
            xTaskCreate(imu_task_entry, "imu", 4096, this, 1, nullptr);
            xTaskCreate(controller_task_entry, "ctrl", 8192, this, 2, nullptr);
            xTaskCreate(led_task_entry, "led", 4096, this, 1, nullptr);
            xTaskCreate(speaker_task_entry, "speaker", 8192, this, 1, nullptr);

            LOGI(TAG, "SmartSignage setup complete");
        }

        void SmartSignage::loop()
        {
            // ESPHome will call this, but all work is in FreeRTOS tasks
            vTaskDelay(pdMS_TO_TICKS(1000));
        }

        void SmartSignage::dump_config()
        {
            LOGI(TAG, "SmartSignage component loaded");
        }

        // --- Task thunks invoking FSMs ---
        void SmartSignage::radar_task_entry(void *pv)
        {
            auto *self = static_cast<SmartSignage *>(pv);
            for (;;)
            {
                self->radar_fsm_.process();
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }

        void SmartSignage::imu_task_entry(void *pv)
        {
            auto *self = static_cast<SmartSignage *>(pv);
            for (;;)
            {
                self->imu_fsm_.process();
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }

        void SmartSignage::controller_task_entry(void *pv)
        {
            auto *self = static_cast<SmartSignage *>(pv);
            for (;;)
            {
                self->controller_fsm_.process();
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }

        void SmartSignage::led_task_entry(void *pv)
        {
            auto *self = static_cast<SmartSignage *>(pv);
            for (;;)
            {
                self->led_fsm_.process();
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }

        void SmartSignage::speaker_task_entry(void *pv)
        {
            auto *self = static_cast<SmartSignage *>(pv);
            for (;;)
            {
                self->speaker_fsm_.process();
                vTaskDelay(pdMS_TO_TICKS(1000));
            }
        }

    } // namespace smart_signage
} // namespace esphome
