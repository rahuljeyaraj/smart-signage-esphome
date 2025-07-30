#include "smart_signage.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "message_protocol.h"
// #include <boost/sml.hpp>

namespace esphome
{
    namespace smart_signage
    {

        // --- Component methods ---
        void SmartSignage::setup()
        {
            // Create buffers (64 bytes each for now)
            controller_queue_ = xQueueCreate(10, sizeof(Event));
            led_queue_ = xQueueCreate(5, sizeof(Command));
            speaker_queue_ = xQueueCreate(5, sizeof(Command));

            // Spawn tasks
            xTaskCreate(radar_task_entry, "radar", 4096, this, 1, nullptr);
            xTaskCreate(controller_task_entry, "ctrl", 8192, this, 2, nullptr);
            xTaskCreate(led_task_entry, "led", 4096, this, 1, nullptr);
            xTaskCreate(speaker_task_entry, "speaker", 8192, this, 1, nullptr);

            LOGI(TAG, "SmartSignage setup complete");
        }

        void SmartSignage::loop() {}

        void SmartSignage::dump_config()
        {
            LOGI(TAG, "SmartSignage component loaded");
        }

        // ─── Radar stub: alternates Presence/Fall every 5s ───────────────────────────
        void SmartSignage::radar_task_entry(void *pv)
        {
            auto *self = static_cast<SmartSignage *>(pv);
            Event e;
            bool toggle = false;

            for (;;)
            {
                e.id = toggle ? EventId::PresenceDetected : EventId::FallDetected;
                e.timestamp = xTaskGetTickCount();
                e.data = etl::monostate{};

                LOGI(TAG, "sending %s",
                     e.id == EventId::PresenceDetected ? "PresenceDetected" : "FallDetected");
                xQueueSend(self->controller_queue_, &e, portMAX_DELAY);

                toggle = !toggle;
                vTaskDelay(pdMS_TO_TICKS(5000));
            }
        }

        // ─── Controller stub: dispatches one Command per Event ───────────────────────
        void SmartSignage::controller_task_entry(void *pv)
        {
            auto *self = static_cast<SmartSignage *>(pv);
            Event e;
            Command cmd;

            for (;;)
            {
                if (xQueueReceive(self->controller_queue_, &e, portMAX_DELAY) == pdPASS)
                {
                    if (e.id == EventId::PresenceDetected)
                    {
                        LOGI(TAG, "PresenceDetected → LedOn(80)");
                        cmd.id = CmdId::LedOn;
                        cmd.param = uint8_t(80);
                        xQueueSend(self->led_queue_, &cmd, portMAX_DELAY);
                    }
                    else
                    {
                        LOGI(TAG, "FallDetected → PlayAudio(0)");
                        cmd.id = CmdId::PlayAudio;
                        cmd.param = uint16_t(0);
                        xQueueSend(self->speaker_queue_, &cmd, portMAX_DELAY);
                    }
                }
            }
        }

        // ─── LED consumer stub ────────────────────────────────────────────────────────
        void SmartSignage::led_task_entry(void *pv)
        {
            auto *self = static_cast<SmartSignage *>(pv);
            Command cmd;
            for (;;)
            {
                if (xQueueReceive(self->led_queue_, &cmd, portMAX_DELAY) == pdPASS)
                {
                    switch (cmd.id)
                    {
                    case CmdId::LedOn:
                        LOGI(TAG, "LedOn, brightness=%u", etl::get<uint8_t>(cmd.param));
                        break;
                    default:
                        LOGI(TAG, "unknown cmd");
                    }
                }
            }
        }

        // ─── Speaker consumer stub ────────────────────────────────────────────────────
        void SmartSignage::speaker_task_entry(void *pv)
        {
            auto *self = static_cast<SmartSignage *>(pv);
            Command cmd;
            for (;;)
            {
                if (xQueueReceive(self->speaker_queue_, &cmd, portMAX_DELAY) == pdPASS)
                {
                    switch (cmd.id)
                    {
                    case CmdId::PlayAudio:
                        LOGI(TAG, "PlayAudio, fileIndex=%u", etl::get<uint16_t>(cmd.param));
                        break;
                    default:
                        LOGI(TAG, "unknown cmd");
                    }
                }
            }
        }

        void SmartSignage::on_start_button()
        {
            LOGI(TAG, "Start button pressed — radius=%.2f m, duration=%u s, vol=%u, bri=%u",
                 settings_.radius_m, settings_.duration_s,
                 settings_.volume, settings_.brightness);
            // dispatch start event to FSM here...
        }

    } // namespace smart_signage
} // namespace esphome
