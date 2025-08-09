#pragma once

#include "active_object.h"
#include "imu/imu_q.h"
#include "ctrl/ctrl_q.h"
#include "imu/imu_fsm.h"
#include "timer/itimer.h"
#include "fsm_logger.h"
#include "log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace esphome::smart_signage::imu {

class ImuAO : public ActiveObject<Q, FSM> {
  public:
    explicit ImuAO(Q &ownQ, ctrl::Q &ctrlQ, hal::IImuHal &hal, timer::ITimer &timer,
        const char *taskName, uint32_t stackSize = 8192,
        UBaseType_t priority = tskIDLE_PRIORITY + 1, BaseType_t coreId = tskNO_AFFINITY)
        : ActiveObject<Q, FSM>(ownQ, fsm_, s_logger, taskName, stackSize, priority, coreId),
          fsm_(ctrlQ, hal, timer), timer_(timer) {
        if (!timer_.create(taskName, &ImuAO::timerCbStatic, this)) {
            SS_LOGE("Failed to create polling timer");
        }
    }

  private:
    FSM            fsm_;
    timer::ITimer &timer_;

    // single logger shared by all ImuAO instances
    inline static FsmLogger s_logger{"ImuFSMLog"};

    // static to member trampoline for ITimer
    static void timerCbStatic(void *arg) { static_cast<ImuAO *>(arg)->onTimerCb(); }

    // called in the timer task context: enqueue EvtTimerPoll on imu::Q
    void onTimerCb() { queue_.post(EvtTimerPoll{}); }

    static constexpr char TAG[] = "ImuAO";
};

} // namespace esphome::smart_signage::imu
