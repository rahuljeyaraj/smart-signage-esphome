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
    explicit RadarAO(Q &ownQ, ctrl::Q &ctrlQ, hal::IRadarHal &hal, timer::ITimer &timer,
        const char *taskName, uint32_t stackSize = 8192,
        UBaseType_t priority = tskIDLE_PRIORITY + 1, BaseType_t coreId = tskNO_AFFINITY)
        : ActiveObject<Q, FSM>(ownQ, fsm_, s_logger, taskName, stackSize, priority, coreId),
          fsm_(ctrlQ, hal, timer), timer_(timer) {
        if (!timer_.create(taskName, &RadarAO::timerCbStatic, this)) {
            LOGE("Failed to create polling timer");
        }
    }

  private:
    FSM            fsm_;
    timer::ITimer &timer_;

    // single logger shared by all RadarAO instances
    inline static FsmLogger s_logger{"RadarFSMLog"};

    // static to member trampoline for ITimer
    static void timerCbStatic(void *arg) { static_cast<RadarAO *>(arg)->onTimerCb(); }

    // called in the timer task context: enqueue EvtTimerPoll on radar::Q
    void onTimerCb() { queue_.post(EvtTimerPoll{}); }

    static constexpr char TAG[] = "RadarAO";
};

} // namespace esphome::smart_signage::radar
