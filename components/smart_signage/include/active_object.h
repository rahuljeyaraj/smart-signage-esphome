#pragma once
#include "log.h"
#include "fsm_logger.h"
#include "sml.hpp"
#include <etl/variant.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace esphome::smart_signage {

namespace sml = boost::sml;

/**
 * Generic Active-Object wrapper
 *   • owns an SML state-machine            (fsm_)
 *   • consumes events from a FreeRTOS queue (queue_)
 *   • runs in its own task                  (taskHandle_)
 */
template <typename QueueType, typename Functor>
class ActiveObject {
  public:
    using EventType = typename QueueType::ItemType; // ← ↑ added *typename*

    explicit ActiveObject(QueueType &queue, Functor &functor, FsmLogger &fsmLogger,
        const char *taskName, // only one string now
        uint32_t stackSize = 2048, UBaseType_t priority = tskIDLE_PRIORITY + 1,
        BaseType_t coreId = tskNO_AFFINITY)
        : TAG{taskName}, queue_{queue}, fsm_{functor, fsmLogger} {
        if (xTaskCreatePinnedToCore(&ActiveObject::taskEntry,
                taskName,
                stackSize,
                this,
                priority,
                &taskHandle_,
                coreId) != pdPASS) {
            LOGE("Task creation failed");
        }
    }

    bool post(const EventType &e, TickType_t wait = 0) {
        return queue_.post(e, wait); // forward to queue
    }

  private:
    static void taskEntry(void *pv) { static_cast<ActiveObject *>(pv)->run(); }

    void run() {
        EventType evt;
        for (;;) {
            if (xQueueReceive(queue_.handle(), &evt, portMAX_DELAY) == pdPASS) {
                etl::visit([this](auto const &ev) { fsm_.process_event(ev); }, evt);
            }
        }
    }

    const char                              *TAG;
    QueueType                               &queue_;
    sml::sm<Functor, sml::logger<FsmLogger>> fsm_;
    TaskHandle_t                             taskHandle_{nullptr};
};

} // namespace esphome::smart_signage
