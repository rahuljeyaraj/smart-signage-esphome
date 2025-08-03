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
 * A generic “active object” that
 *  - wraps an SML functor internally,
 *  - creates its own FreeRTOS task in the ctor,
 *  - and pushes Events (std::variant) into the FSM.
 */
template <typename QueueType, typename Functor, const char *TAG>
class ActiveObject {
  public:
    // deduce EventType from the QueueType
    using EventType = QueueType::ItemType;

    explicit ActiveObject(QueueType &queue,
        Functor                     &functor,
        FsmLogger                   &fsmLogger,
        const char                  *taskName  = "default_task",
        uint32_t                     stackSize = 2048,
        UBaseType_t                  priority  = tskIDLE_PRIORITY + 1,
        BaseType_t                   coreId    = 0)
        : queue_(queue), fsm_(functor, fsmLogger) {
        if (xTaskCreatePinnedToCore(&ActiveObject::taskEntry,
                taskName,
                stackSize,
                this,
                priority,
                &taskHandle_,
                coreId) != pdPASS) {
            LOGE(TAG, "Task creation failed");
        }
    }

    bool post(const EventType &e, TickType_t wait = 0) {
        // forward to the queue’s own post
        return queue_.post(e, wait);
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

    sml::sm<Functor, sml::logger<FsmLogger>> fsm_;
    // sml::sm<Functor> fsm_;

    QueueType   &queue_;
    TaskHandle_t taskHandle_{nullptr};
};

} // namespace esphome::smart_signage
