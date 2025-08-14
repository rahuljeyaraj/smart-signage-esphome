// #pragma once
// #include "active_object.h"
// #include "led_q.h"
// #include "led_fsm.h"
// #include "led/hal/iled_hal.h"
// #include "timer/itimer.h"
// #include "log.h"

// namespace esphome::smart_signage::led {

// static const EvtFadeEnd kEvtFadeEnd{};

// class LedAO : public ActiveObject<Q, FSM> {
//   public:
//     LedAO(Q &ownQ, ctrl::Q &ctrlQ, hal::ILedHal &hal, timer::ITimer &timer,
//         const char *taskName = "led", uint32_t stack = 4096,
//         UBaseType_t prio = tskIDLE_PRIORITY + 1, BaseType_t coreId = tskNO_AFFINITY)
//         : ActiveObject<Q, FSM>(ownQ, fsm_, taskName, stack, prio), logger_{taskName},
//           fsm_(ctrlQ, hal, timer), timer_(timer) {

//         // HAL signals fade complete -> our queue
//         hal.setFadeEndCallback(&LedAO::fadeCbThunk, this);

//         this->start(logger_, coreId);
//         SS_LOGI("LedAO started");
//     }

//   private:
//     static void IRAM_ATTR fadeCbThunk(void *arg) {
//         auto      *self = static_cast<LedAO *>(arg);
//         BaseType_t hpw  = pdFALSE;
//         self->queue_.postFromISR(kEvtFadeEnd, &hpw);
//         if (hpw) portYIELD_FROM_ISR();
//     }

//     FsmLogger      logger_;
//     FSM            fsm_;
//     timer::ITimer &timer_;

//     static constexpr char TAG[] = "CtrlAO";
// };

// } // namespace esphome::smart_signage::led
