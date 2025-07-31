#include "smart_signage.h"

#include <etl/variant.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include <string>

#include "logger.h"
#include "message_protocol.h"
#include "sml.hpp"

#define EXPECT_STATE(SM, STATE_TAG)                                                                \
    if (!(SM).is(sml::state<STATE_TAG>)) {                                                         \
        LOGE("FSM", "Unexpected state! Wanted: %s", #STATE_TAG);                                   \
    } else {                                                                                       \
        LOGI("FSM", "Got expected state: %s", #STATE_TAG);                                         \
    }

namespace esphome::smart_signage {

namespace sml = boost::sml;
using namespace sml::literals;

// queue_ = xQueueCreate(QLEN, sizeof(Event));
// xTaskCreatePinnedToCore(&ActiveObject::taskEntry, task_name, stack_size,
//                         this, // pvParameters
//                         priority, &task_handle_, core_id);

// template <class Fsm, typename Event, size_t QLEN = 8> class ActiveObject {
//   public:
//     ActiveObject(// inject fsm, queue, task) {

//     }

//     bool post(const Event &e, TickType_t ticks_to_wait = 0) {
//         return xQueueSend(queue_, &e, ticks_to_wait) == pdPASS;
//     }

//     virtual ~ActiveObject() = default;

//   protected:
//     void on_event(const Event &e) {
//         etl::visit([this](auto const &ev) { fsm_.process_event(ev); }, e);
//     }

//   private:
//     static void taskEntry(void *pv) { static_cast<ActiveObject *>(pv)->run(); }

//     void run() {
//         Event e;
//         for (;;) {
//             if (xQueueReceive(queue_, &e, portMAX_DELAY) == pdPASS) {
//                 on_event(e);
//             }
//         }
//     }

//     QueueHandle_t queue_{nullptr};
//     TaskHandle_t task_handle_{nullptr};
//     Fsm fsm_;
// };

// template <class Fsm, typename Event, size_t QLEN = 8>
// class FsmActiveObject : public ActiveObject<Event, QLEN> {
//   public:
//     /* Forward FSM constructor args through this AO’s ctor */
//     template <class... Args>
//     FsmActiveObject(const char *task_name, UBaseType_t prio = tskIDLE_PRIORITY + 1,
//                     uint32_t stack_words = 4096, BaseType_t core_id = tskNO_AFFINITY,
//                     Args &&...fsm_args)
//         : ActiveObject<Event, QLEN>(task_name, prio, stack_words, core_id),
//           fsm_{std::forward<Args>(fsm_args)...} // build the FSM here
//     {}

//   protected:
//     /* Called by ActiveObject’s worker task whenever a message arrives */
//     void on_event(const Event &e) override {
//         etl::visit([this](auto const &ev) { fsm_.process_event(ev); }, e);
//     }

//   private:
//     Fsm fsm_;
// };

template <typename Functor, typename Event> class ActiveObject {
  public:
    // Now you pass in your RadarFSM (the functor) instead of the SML wrapper
    ActiveObject(Functor &functor, QueueHandle_t queue, const char *taskName,
                 uint32_t stackSize = 2048, UBaseType_t priority = tskIDLE_PRIORITY + 1,
                 BaseType_t coreId = 0)
        : fsm_{functor}, queue_(queue) {
        if (xTaskCreatePinnedToCore(&ActiveObject::taskEntry, taskName, stackSize, this, priority,
                                    &taskHandle_, coreId) != pdPASS) {
            // handle error…
            // e.g. LOGE("AO", "Task creation failed");
        }
    }

    bool post(const Event &e, TickType_t wait = 0) {
        return xQueueSend(queue_, &e, wait) == pdPASS;
    }

  private:
    // FreeRTOS entry point
    static void taskEntry(void *pv) {
        auto *self = static_cast<ActiveObject *>(pv);
        self->run();
    }

    // Endless loop that pumps events into the SML wrapper
    void run() {
        Event evt;
        for (;;) {
            if (xQueueReceive(queue_, &evt, portMAX_DELAY) == pdPASS) {
                etl::visit([this](auto const &ev) { fsm_.process_event(ev); }, evt);
            }
        }
    }

    // **Here** is the big change: we hold a `boost::sml::sm<Functor>` internally
    sml::sm<Functor> fsm_;
    QueueHandle_t queue_;
    TaskHandle_t taskHandle_{nullptr};
};

struct Configure {
    uint16_t detDistCm;
    uint32_t sampleIntMs;
};
struct Activate {};
struct Deactivate {};
struct TimerPoll {};
/*---------------------------------- FSM -------------------------------------*/
class RadarFSM {
    using Self = RadarFSM; // handy alias for &Self::action
  public:
    /*---------------- Shared data carried by the FSM instance ---------------*/
    uint16_t detDistCm_ = 0;
    uint32_t sampleIntMs_ = 0;

    /*---------------- Boost.SML definition ----------------------------------*/
    auto operator()() {
        using namespace sml;

        return make_transition_table(
            // Idle → Ready after configuration
            *state<Idle> + event<Configure> / &Self::act_configure = state<Ready>,

            // Ready → Active on Activate
            state<Ready> + event<Activate> / &Self::act_activate = state<Active>,

            // Active : poll timer keeps machine in Active
            state<Active> + event<TimerPoll> / &Self::act_poll = state<Active>,

            // Active → Ready on Deactivate
            state<Active> + event<Deactivate> / &Self::act_deactivate = state<Ready> // last
        );
    }

    /*---------------- Actions (and optional guards) -------------------------*/
    void act_configure(const Configure &cfg) {
        detDistCm_ = cfg.detDistCm;
        sampleIntMs_ = cfg.sampleIntMs;
        LOGI("RadarFSM", "Configured: dist=%u cm, interval=%u ms", detDistCm_, sampleIntMs_);
    }

    void act_activate(const Activate &) { LOGI("RadarFSM", "Activated"); }

    void act_poll(const TimerPoll &) {
        // In real code, read the sensor here …
        LOGI("RadarFSM", "Polling… (maxDist=%u cm, Δt=%u ms)", detDistCm_, sampleIntMs_);
    }

    void act_deactivate(const Deactivate &) { LOGI("RadarFSM", "Deactivated"); }

  private:
    // State tags kept private to the class
    struct Idle {};
    struct Ready {};
    struct Active {};
};

/* 2.  Event variant shared with the AO */
using RadarEvent = etl::variant<Configure, Activate, Deactivate, TimerPoll>;

RadarFSM radarFsm;
auto radarQueue = xQueueCreate(8, sizeof(RadarEvent));

// 2) Construct ActiveObject: it will create its own task immediately
ActiveObject<RadarFSM, RadarEvent> radarDriver{
    radarFsm,
    radarQueue,
    "radarTask", // task name
    8192,        // stack size
    tskIDLE_PRIORITY + 2,
    1 // pinned to core 1 on dual-core chips
};

// RadarAO radar;
void SmartSignage::setup() {

    // Create an instance of your class

    // Inject the instance

    // Now you can process events, and the actions will be executed on `my_instance`
}

void SmartSignage::loop() {
    radarDriver.post(Configure{200, 1000});
    radarDriver.post(Activate{});
    radarDriver.post(TimerPoll{});
    radarDriver.post(Deactivate{});
    // sm.process_event(Activate{});
    // sm.process_event(Configure{10, 20});
    vTaskDelay(pdMS_TO_TICKS(100));
}

// // 2) activate  → Ready → Active
// radar.post(Activate{});
// vTaskDelay(pdMS_TO_TICKS(10));

// // 3) simulate polls
// for (int i = 0; i < 3; ++i) {
//     radar.post(TimerPoll{});
//     vTaskDelay(pdMS_TO_TICKS(10));
// }

// // 4) deactivate → Active → Ready
// radar.post(Deactivate{});
// vTaskDelay(pdMS_TO_TICKS(10));
// }
void SmartSignage::dump_config() { LOGI(TAG, "component loaded"); }

} // namespace esphome::smart_signage
