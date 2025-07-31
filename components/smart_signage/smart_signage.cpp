#include "smart_signage.h"
#include "message_protocol.h"
#include "logger.h"

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

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
        struct release
        {
        };
        struct ack
        {
        };
        struct fin
        {
        };
        struct timeout
        {
        };

        // guards
        const auto is_ack_valid = [](const ack &)
        { return true; };
        const auto is_fin_valid = [](const fin &)
        { return true; };

        // actions
        const auto send_fin = [] {};
        const auto send_ack = [] {};

        // states
        class established;
        class fin_wait_1;
        class fin_wait_2;
        class timed_wait;

        struct hello_world
        {
            auto operator()() const
            {
                using namespace sml;
                // clang-format off
                return make_transition_table(
                *state<established> + event<release> / send_fin = state<fin_wait_1>,
                state<fin_wait_1> + event<ack> [ is_ack_valid ] = state<fin_wait_2>,
                state<fin_wait_2> + event<fin> [ is_fin_valid ] / send_ack = state<timed_wait>,
                state<timed_wait> + event<timeout> / send_ack = X
                );
                // clang-format on
            }
        };

        /*----------------------------------------------------------------------------*/
        /* Event                                                                      */
        /*----------------------------------------------------------------------------*/
        struct init
        {
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
                // clang-format off
                return make_transition_table(
                    *state<idle>  + event<init> = state<ready>
                );
                // clang-format on
            }
        };
    }
    /* ------------------------------------------------------------------ */
    /*  Demo driver                                                       */
    /* ------------------------------------------------------------------ */

    void SmartSignage::setup()
    {
    }

    void
    SmartSignage::loop()
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        using namespace sml;

        sml::sm<simple_fsm> sm1; // starts in idle
        EXPECT_STATE(sm1, idle);
        sm1.process_event(init{}); // -> ready
        EXPECT_STATE(sm1, ready);

        sm<hello_world> sm;
        EXPECT_STATE(sm, established);

        sm.process_event(release{});
        EXPECT_STATE(sm, fin_wait_1);

        sm.process_event(ack{});
        EXPECT_STATE(sm, fin_wait_2);

        sm.process_event(fin{});
        EXPECT_STATE(sm, timed_wait);

        sm.process_event(timeout{});
        if (!sm.is(X))
        {
            LOGE("FSM", "Error");
        }
        else
        {
            LOGI("FSM", "Sucess");
        }
    }
    void SmartSignage::dump_config() { LOGI(TAG, "component loaded"); }

} // namespace esphome::smart_signage
