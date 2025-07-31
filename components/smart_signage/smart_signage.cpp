#include "smart_signage.h"
#include "message_protocol.h"
#include "logger.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <etl/variant.h>

#include "sml.hpp"

#include "logger.h"

#include <string>

#define EXPECT_STATE(SM, STATE_TAG)                              \
                                                                 \
    if (!(SM).is(sml::state<STATE_TAG>))                         \
    {                                                            \
        LOGE("FSM", "Unexpected state! Wanted: %s", #STATE_TAG); \
    }                                                            \
    else                                                         \
    {                                                            \
        LOGI("FSM", "Got expected state: %s", #STATE_TAG);       \
    }

namespace esphome::smart_signage
{
    // ────────────────────────────────────────────────────────────────────────────
    //  YAML-driven setters (unchanged)
    // ────────────────────────────────────────────────────────────────────────────
    void SmartSignage::set_radius(float v)
    {
        settings_.radius_m = v;
        LOGI(TAG, "radius %.2f", v);
    }
    void SmartSignage::set_duration(float v)
    {
        settings_.duration_s = v;
        LOGI(TAG, "duration %u", settings_.duration_s);
    }
    void SmartSignage::set_volume(float v)
    {
        settings_.volume = v;
        LOGI(TAG, "volume %u", settings_.volume);
    }
    void SmartSignage::set_brightness(float v)
    {
        settings_.brightness = v;
        LOGI(TAG, "brightness %u", settings_.brightness);
    }
    void SmartSignage::on_start_button()
    {
        LOGI(TAG, "Start btn pressed"); /* later: g_fsm.receive(StartBtnMsg{}) */
    }

    namespace sml = boost::sml;
    using namespace sml::literals;

    struct Presence
    {
        bool detected; // true = somebody near sensor
        void message() const
        {
            LOGI("EVENT", "Radar says: %s", detected ? "PRESENT" : "CLEAR");
        }
    };

    /* Add more events later … */
    using Event = etl::variant<Presence>;

    /***********************************************************************/
    /* 2.  Controller FSM                                                  */
    /***********************************************************************/
    namespace sml = boost::sml;

    struct ControllerFsm
    {
        using self = ControllerFsm;
        /* state tags */
        struct idle
        {
        };
        struct busy
        {
        };
        // const bool presence_detected = [](const Presence &e)
        // {
        //     return (e.detected);
        // };
        // const bool presence_cleared = [](const Presence &e)
        // {
        //     return !e.detected;
        // };

        auto operator()() const
        {
            using namespace sml;

            const auto log_enter_idle = []
            { LOGI("FSM", "→ idle"); };
            const auto log_enter_busy = []
            { LOGI("FSM", "→ busy"); };

            // clang-format off
            return make_transition_table(
                *state<idle>  + event<Presence> [ ([](const Presence& e){ return e.detected; } )]
                                / log_enter_busy = state<busy>,

                state<busy>  + event<Presence> [ ([](const Presence& e){ return !e.detected; } )]
                                / log_enter_idle = state<idle>
            );
            // clang-format on
        }
    };

    /***********************************************************************/
    /* 3.  Controller active object                                        */
    /***********************************************************************/
    class ControllerAO
    {
    public:
        explicit ControllerAO(size_t qlen = 8)
        {
            queue_ = xQueueCreate(qlen, sizeof(Event));
        }

        QueueHandle_t queue() const { return queue_; }

        void start(UBaseType_t prio = 5, uint32_t stack = 4096, BaseType_t core = 1)
        {
            xTaskCreatePinnedToCore(taskThunk, "ctrl_task", stack, this, prio, &task_, core);
        }

    private:
        static void taskThunk(void *arg) { static_cast<ControllerAO *>(arg)->run(); }

        void run()
        {
            sml::sm<ControllerFsm> sm;
            Event ev;

            while (true)
            {
                if (xQueueReceive(queue_, &ev, portMAX_DELAY) == pdPASS)
                {
                    etl::visit([&](auto &e)
                               {
                                   e.message();         // generic per-event print
                                   sm.process_event(e); // feed FSM
                               },
                               ev);
                }
            }
        }

        QueueHandle_t queue_{nullptr};
        TaskHandle_t task_{nullptr};
    };

    /***********************************************************************/
    /* 4.  Radar active object                                             */
    /***********************************************************************/
    class RadarAO
    {
    public:
        explicit RadarAO(ControllerAO &ctrl) : ctrlQ_{ctrl.queue()} {}

        void start(UBaseType_t prio = 4, uint32_t stack = 2048, BaseType_t core = 1)
        {
            xTaskCreatePinnedToCore(taskThunk, "radar_task", stack, this, prio, &task_, core);
        }

    private:
        static void taskThunk(void *arg) { static_cast<RadarAO *>(arg)->run(); }

        void run()
        {
            const TickType_t period = pdMS_TO_TICKS(2000);
            bool detected = false;

            for (;;)
            {
                Event ev = Presence{detected};
                xQueueSend(ctrlQ_, &ev, 0); // never block; controller must be fast
                detected = !detected;       // toggle every cycle
                vTaskDelay(period);
            }
        }

        QueueHandle_t ctrlQ_;
        TaskHandle_t task_{nullptr};
    };

    ControllerAO controller; // lifetime = whole program
    RadarAO radar(controller);

    void SmartSignage::setup()
    {
        controller.start();
        radar.start();
    }

    void SmartSignage::loop()
    {
    }
    void SmartSignage::dump_config() { LOGI(TAG, "component loaded"); }

} // namespace esphome::smart_signage
