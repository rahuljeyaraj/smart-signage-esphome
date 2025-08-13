#pragma once
#include "active_object.h"
#include "ctrl/ctrl_q.h"
#include "ctrl/ctrl_fsm.h"
#include "fsm_logger.h"
#include "log.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace esphome::smart_signage::ctrl {

class CtrlAO : public ActiveObject<Q, FSM> {
  public:
    explicit CtrlAO(Q &ownQ, radar::Q &radarQ, imu::Q &imuQ, led::Q &ledQ, audio::Q &audioQ,
        timer::ITimer &timer, ProfilesConfigT &cfg, UserIntfT &ui, const char *taskName,
        uint32_t stackSize = 8192, UBaseType_t priority = tskIDLE_PRIORITY + 1,
        BaseType_t coreId = tskNO_AFFINITY)
        : ActiveObject<Q, FSM>(ownQ, fsm_, s_logger, taskName, stackSize, priority, coreId),
          fsm_(radarQ, imuQ, ledQ, audioQ, timer, cfg, ui), timer_(timer) {

        if (!timer_.create(taskName, &CtrlAO::timerCbStatic, this)) {
            SS_LOGE("CtrlAO timer create failed");
        }
    }

  private:
    FSM            fsm_;
    timer::ITimer &timer_;

    // single logger shared by all CtrlAO instances
    inline static FsmLogger s_logger{"CtrlFSMLog"};

    // static-to-member trampoline for ITimer
    static void timerCbStatic(void *arg) { static_cast<CtrlAO *>(arg)->onTimerCb(); }

    // called in the timer task context: enqueue EvtTimerTick on ctrl::Q
    void onTimerCb() { this->queue_.post(EvtTimerEnd{}); }

    static constexpr char TAG[] = "CtrlAO";
};

} // namespace esphome::smart_signage::ctrl
