#pragma once

#include "active_object.h"
#include "radar/radar_q.h"
#include "ctrl/ctrl_q.h"
#include "radar/radar_fsm.h"
#include "timer/itimer.h"
#include "fsm_logger.h"
#include "log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace esphome::smart_signage::radar {

class RadarAO : public ActiveObject<Q, FSM> {
  public:
    explicit RadarAO( // clang-format off
      Q &ownQ,
      FSM radarFsm,
      FsmLogger radarFsmlogger,
      const char *taskName,
      uint32_t stackSize = 8192,
      UBaseType_t priority = tskIDLE_PRIORITY + 1,
      BaseType_t coreId = tskNO_AFFINITY
    ) // clang-format on
        : ActiveObject<Q, FSM>(
              ownQ, radarFsm, radarFsmlogger, taskName, stackSize, priority, coreId) {}
    // if (!timer_.create(taskName, &RadarAO::timerCbStatic, this)) {
    //     SS_LOGE("Failed to create polling timer");
    // }
    // }

  private:
    // // static to member trampoline for ITimer
    // static void timerCbStatic(void *arg) { static_cast<RadarAO *>(arg)->onTimerCb(); }

    // // called in the timer task context: enqueue EvtTimerPoll on radar::Q
    // void onTimerCb() { queue_.post(EvtTimerPoll{}); }

    static constexpr char TAG[] = "RadarAO";
};

} // namespace esphome::smart_signage::radar
