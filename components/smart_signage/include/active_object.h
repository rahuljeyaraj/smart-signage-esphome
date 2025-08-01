#pragma once

#include "log.h"
#include "sml.hpp"
#include <etl/variant.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

namespace esphome::smart_signage {

namespace sml = boost::sml;

/**
 * A generic “active object” that
 *  - wraps an SML functor internally,
 *  - creates its own FreeRTOS task in the ctor,
 *  - and pushes Events (std::variant) into the FSM.
 */
template <typename Functor, typename Event> class ActiveObject {
  public:
    ActiveObject(Functor &functor, QueueHandle_t queue, const char *taskName,
                 uint32_t stackSize = 2048, UBaseType_t priority = tskIDLE_PRIORITY + 1,
                 BaseType_t coreId = 0)
        : fsm_{functor}, queue_{queue} {
        if (xTaskCreatePinnedToCore(&ActiveObject::taskEntry, taskName, stackSize, this, priority,
                                    &taskHandle_, coreId) != pdPASS) {
            LOGE("AO", "Task creation failed");
        }
    }

    /** Post an event into the queue for processing. */
    bool post(const Event &e, TickType_t wait = 0) {
        return xQueueSend(queue_, &e, wait) == pdPASS;
    }

  private:
    static void taskEntry(void *pv) { static_cast<ActiveObject *>(pv)->run(); }

    void run() {
        Event evt;
        for (;;) {
            if (xQueueReceive(queue_, &evt, portMAX_DELAY) == pdPASS) {
                etl::visit([this](auto const &ev) { fsm_.process_event(ev); }, evt);
            }
        }
    }

    sml::sm<Functor> fsm_;
    QueueHandle_t queue_;
    TaskHandle_t taskHandle_{nullptr};
};

} // namespace esphome::smart_signage
