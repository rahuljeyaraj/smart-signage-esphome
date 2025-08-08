#pragma once

#include "active_object.h"
#include "led/led_q.h"
#include "ctrl/ctrl_q.h"
#include "led/led_fsm.h"
#include "timer/itimer.h"
#include "fsm_logger.h"
#include "log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace esphome::smart_signage::led {

class LedAO : public ActiveObject<Q, FSM> {
  public:
    explicit LedAO(Q &ownQ, ctrl::Q &ctrlQ, hal::ILedHal &hal, timer::ITimer &timer,
        const char *taskName, uint32_t stackSize = 8192,
        UBaseType_t priority = tskIDLE_PRIORITY + 1, BaseType_t coreId = tskNO_AFFINITY)
        : ActiveObject<Q, FSM>(ownQ, fsm_, s_logger, taskName, stackSize, priority, coreId),
          fsm_(ctrlQ, hal, timer), timer_(timer) {
        if (!timer_.create(taskName, &LedAO::timerCbStatic, this)) {
            LOGE("Failed to create polling timer");
        }
    }

  private:
    FSM            fsm_;
    timer::ITimer &timer_;

    // single logger shared by all LedAO instances
    inline static FsmLogger s_logger{"LedFSMLog"};

    // static to member trampoline for ITimer
    static void timerCbStatic(void *arg) { static_cast<LedAO *>(arg)->onTimerCb(); }

    // called in the timer task context: enqueue EvtTimerPoll on led::Q
    void onTimerCb() { queue_.post(EvtTimerPoll{}); }

    static constexpr char TAG[] = "LedAO";
};

} // namespace esphome::smart_signage::led
