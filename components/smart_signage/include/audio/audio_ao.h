// #pragma once
// #include "active_object.h"
// #include "audio_q.h"
// #include "audio_fsm.h"
// #include "audio/hal/iaudio_hal.h"
// #include "timer/itimer.h"
// #include "log.h"

// namespace esphome::smart_signage::audio {

// class AudioAO : public ActiveObject<Q, FSM> {
//   public:
//     AudioAO(Q &ownQ, ctrl::Q &ctrlQ, hal::IAudioHal &hal, timer::ITimer &timer,
//         const char *taskName = "audio", uint32_t stack = 6144,
//         UBaseType_t prio = tskIDLE_PRIORITY + 1, BaseType_t coreId = tskNO_AFFINITY)
//         : ActiveObject<Q, FSM>(ownQ, fsm_, taskName, stack, prio), logger_{taskName},
//           fsm_(ctrlQ, hal, timer), timer_(timer) {

//         // hook audio HAL callbacks to our queue if exposed

//         this->start(logger_, coreId);
//         SS_LOGI("AudioAO started");
//     }

//   private:
//     FsmLogger      logger_;
//     FSM            fsm_;
//     timer::ITimer &timer_;

//     static constexpr char TAG[] = "CtrlAO";
// };

// } // namespace esphome::smart_signage::audio
