// #pragma once
// #include "active_object.h"
// #include "imu_q.h"
// #include "imu_fsm.h"
// #include "imu/hal/iimu_hal.h"
// #include "timer/itimer.h"
// #include "log.h"

// namespace esphome::smart_signage::imu {

// static const EvtTimerPoll kPollEvt{};

// class ImuAO : public ActiveObject<Q, FSM> {
//   public:
//     ImuAO(Q &ownQ, ctrl::Q &ctrlQ, hal::IImuHal &hal, timer::ITimer &timer,
//         const char *taskName = "imu", uint32_t stack = 6144,
//         UBaseType_t prio = tskIDLE_PRIORITY + 1, BaseType_t coreId = tskNO_AFFINITY)
//         : ActiveObject<Q, FSM>(ownQ, fsm_, taskName, stack, prio), logger_{taskName},
//           fsm_(ctrlQ, hal, timer), timer_(timer) {

//         if (!timer_.create(taskName, &ImuAO::timerCbThunk, this)) {
//             SS_LOGE("imu timer create fail");
//         }

//         this->start(logger_, coreId);
//         SS_LOGI("ImuAO started");
//     }

//   private:
//     static void IRAM_ATTR timerCbThunk(void *arg) {
//         auto      *self = static_cast<ImuAO *>(arg);
//         BaseType_t hpw  = pdFALSE;
//         self->queue_.postFromISR(kPollEvt, &hpw);
//         if (hpw) portYIELD_FROM_ISR();
//     }

//     FsmLogger      logger_;
//     FSM            fsm_;
//     timer::ITimer &timer_;

//     static constexpr char TAG[] = "CtrlAO";

// } // namespace esphome::smart_signage::imu
