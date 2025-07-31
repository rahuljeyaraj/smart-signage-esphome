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

    namespace
    {
        // events
        // struct release
        // {
        // };
        // struct ack
        // {
        // };
        // struct fin
        // {
        // };
        // struct timeout
        // {
        // };
        // // guards
        // const auto is_ack_valid = [](const ack &)
        // { return true; };
        // const auto is_fin_valid = [](const fin &)
        // { return true; };
        // // actions
        // const auto send_fin = [] {};
        // const auto send_ack = [] {};
        // // states
        // class established;
        // class fin_wait_1;
        // class fin_wait_2;
        // class timed_wait;
        // struct hello_world
        // {
        //     auto operator()() const
        //     {
        //         using namespace sml;
        //         // clang-format off
        //         return make_transition_table(
        //         *state<established> + event<release> / send_fin = state<fin_wait_1>,
        //         state<fin_wait_1> + event<ack> [ is_ack_valid ] = state<fin_wait_2>,
        //         state<fin_wait_2> + event<fin> [ is_fin_valid ] / send_ack = state<timed_wait>,
        //         state<timed_wait> + event<timeout> / send_ack = X
        //         );
        //         // clang-format on
        //     }
        // };

        /*----------------------------------------------------------------------------*/
        /* Event                                                                      */
        /*----------------------------------------------------------------------------*/
        // struct init
        // {
        // };
        // struct deinit
        // {
        // };

        // Define your structs
        struct init
        {
            void message() const
            {
                LOGI("SS", "Initialization event occurred.");
            }
        };

        struct deinit
        {
            void message() const
            {
                LOGI("SS", "Deinitialization event occurred.");
            }
        };

        /*----------------------------------------------------------------------------*/
        /* States                                                                     */
        /*----------------------------------------------------------------------------*/
        class idle;  // initial state tag
        class ready; // final / running state tag

        /*----------------------------------------------------------------------------*/
        /* Finite-state machine                                                       */
        /*----------------------------------------------------------------------------*/
        struct simple_fsm
        {
            auto operator()() const
            {
                using namespace sml;
                const auto log_init = []
                { ESP_LOGI("FSM", "transition: idle → ready"); };
                const auto log_deinit = []
                { ESP_LOGI("FSM", "transition: ready → idle"); };
                // clang-format off
                return make_transition_table(
                    *state<idle>  + event<init> / log_init= state<ready>,
                    state<ready>  + event<deinit>/ log_deinit = state<idle>
                );
                // clang-format on
            }
        };
    }

    using EventVariant = etl::variant<init, deinit>;

    static QueueHandle_t g_event_q = nullptr;

    static void fsm_task(void *arg)
    {
        sml::sm<simple_fsm> sm;
        EventVariant event;

        while (true)
        {

            if (xQueueReceive(g_event_q, &event, portMAX_DELAY) == pdPASS)
            {
                etl::visit(
                    [&sm](auto &ev) // capture the FSM by reference
                    {
                        ev.message(); // your per-event print/log
                        sm.process_event(ev);
                    },
                    event);
            }
        }
    }

    void SmartSignage::setup()
    {
        g_event_q = xQueueCreate(/*length*/ 8, sizeof(EventVariant));

        // 2. Spawn tasks
        xTaskCreatePinnedToCore(fsm_task, "fsm_task", 4096, nullptr, 5, nullptr, 1);
        // xTaskCreatePinnedToCore(producer_task, "producer_task", 2048, nullptr, 4, nullptr, 1);
    }

    void SmartSignage::loop()
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        EventVariant myEvent;

        myEvent = init{}; // Assign an init event
        // processEvent(myEvent);
        xQueueSend(g_event_q, &myEvent, portMAX_DELAY);

        myEvent = deinit{}; // Assign a deinit event
        // processEvent(myEvent);
        xQueueSend(g_event_q, &myEvent, portMAX_DELAY);

        // xQueueSend(g_event_q, &e1, portMAX_DELAY);
        // xQueueSend(g_event_q, &e2, portMAX_DELAY);
    }
    void SmartSignage::dump_config() { LOGI(TAG, "component loaded"); }

} // namespace esphome::smart_signage
