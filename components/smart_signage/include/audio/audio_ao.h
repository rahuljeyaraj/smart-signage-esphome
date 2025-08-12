#pragma once

#include "active_object.h"
#include "audio/audio_q.h"
#include "ctrl/ctrl_q.h"
#include "audio/audio_fsm.h"
#include "audio/hal/iaudio_hal.h"
#include "timer/itimer.h"
#include "fsm_logger.h"
#include "log.h"

namespace esphome::smart_signage::audio {

class AudioAO : public ActiveObject<Q, FSM> {
  public:
    AudioAO(Q &ownQ, ctrl::Q &ctrlQ, hal::IAudioHAL &hal, timer::ITimer &timer,
        const char *taskName, uint32_t stackSize, UBaseType_t prio, BaseType_t core)
        : ActiveObject<Q, FSM>(ownQ, fsm_, s_logger, taskName, stackSize, prio, core),
          fsm_(ctrlQ, hal, timer), hal_(hal), timer_(timer) {
        // HAL EOF -> post to our queue (task context; not ISR)
        hal_.setPlaybackDoneCallback(&AudioAO::onHalDoneStatic, this);
        // Timer callback posts EvtGapEnd (same pattern as LED AO)
        if (!timer_.create(taskName, &AudioAO::timerCbStatic, this)) {
            SS_LOGE("AudioAO: timer create failed");
        }
    }

  private:
    static void onHalDoneStatic(void *ctx) {
        auto *self = static_cast<AudioAO *>(ctx);
        self->queue_.post(EvtPlaybackDone{});
    }

    static void timerCbStatic(void *arg) {
        auto *self = static_cast<AudioAO *>(arg);
        self->queue_.post(EvtGapEnd{});
    }

    static constexpr char TAG[] = "AudioAO";

    FSM             fsm_;
    hal::IAudioHAL &hal_;
    timer::ITimer  &timer_;

    inline static FsmLogger s_logger{"AudioFSMLog"};
};

using AO = AudioAO;

} // namespace esphome::smart_signage::audio
