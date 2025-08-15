#pragma once

#include "active_object.h"
#include "led/led_q.h"
#include "ctrl/ctrl_q.h"
#include "led/led_fsm.h"
#include "led/hal/iled_hal.h"
#include "timer/itimer.h"
#include "fsm_logger.h"
#include "log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace esphome::smart_signage::led {

static const IRAM_ATTR EvtFadeEnd kEvt{};

namespace {
    static void IRAM_ATTR fadeCbStatic(void *arg) {
        auto *self = static_cast<Q *>(arg);

        BaseType_t hpTaskWoken = pdFALSE;
        self->postFromISR(kEvt, &hpTaskWoken);

        if (hpTaskWoken) portYIELD_FROM_ISR();
    }
} // namespace

class LedAO : public ActiveObject<Q, FSM> {
  public:
    explicit LedAO(                      //
        Q             &ownQ,             // Our own event queue
        ctrl::Q       &ctrlQ,            // Reference to controller queue
        hal::ILedHal  &hal,              // Reference to hardware abstraction layer for LEDs
        timer::ITimer &timer,            // Reference to software/hardware timer
        timer::ITimer &fadeTimer,        // workardound for isr crash
        const char    *taskName,         // Name for this task
        uint32_t       stackSize = 8192, // stackSize
        UBaseType_t    priority  = tskIDLE_PRIORITY + 1, // priority
        BaseType_t     coreId    = tskNO_AFFINITY        // coreId
        )
        : ActiveObject<Q, FSM>(ownQ, fsm_, s_logger, taskName, stackSize, priority, coreId),
          fsm_(ctrlQ, hal, timer, fadeTimer),  //
          timer_(timer), fadeTimer_(fadeTimer) //
    {
        hal.setFadeEndCallback(&fadeCbStatic, &queue_);
        if (!timer_.create(taskName, &timerCbStatic, this)) { SS_LOGE("Failed to create timer"); }
        if (!fadeTimer_.create(taskName, &fadeTimerCbStatic, this)) {
            SS_LOGE("Failed to create fade timer");
        }
    }

  private:
    FSM            fsm_;
    timer::ITimer &timer_;
    timer::ITimer &fadeTimer_;

    // single logger shared by all LedAO instances
    inline static FsmLogger s_logger{"LedFSMLog"};

    static void timerCbStatic(void *arg) {
        auto *self = static_cast<LedAO *>(arg);
        self->queue_.post(EvtTimerEnd{});
    }

    static void fadeTimerCbStatic(void *arg) {
        auto *self = static_cast<LedAO *>(arg);
        self->queue_.post(EvtFadeEnd{});
    }

    static constexpr char TAG[] = "LedAO";
};

} // namespace esphome::smart_signage::led
