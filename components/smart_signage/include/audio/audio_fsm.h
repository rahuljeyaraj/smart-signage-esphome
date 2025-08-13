#pragma once
#include "sml.hpp"
#include "log.h"

#include "ctrl/ctrl_q.h"
#include "ctrl/ctrl_event.h"
#include "audio/audio_event.h"
#include "audio/hal/iaudio_hal.h"
#include "timer/itimer.h"

namespace sml = boost::sml;

namespace esphome::smart_signage::audio {

class FSM {
    using Self = FSM;

  public:
    FSM(ctrl::Q &ctrlQ, hal::IAudioHAL &hal, timer::ITimer &timer)
        : ctrlQ_(ctrlQ), hal_(hal), timer_(timer) {}

    // Transition table must be header-only (ActiveObject pattern).
    auto operator()() noexcept {
        struct Idle {};
        struct Ready {};
        struct Playing {};
        struct Error {};

        using namespace sml;
        return make_transition_table(
            // clang-format off
            *state<Idle>    + event<CmdSetup>         [ &Self::tryInitHal ]  / &Self::onSetupOk    = state<Ready>,
             state<Idle>    + event<CmdSetup>                                / &Self::onSetupFail  = state<Error>,

             state<Ready>   + event<CmdSetVolume>                            / &Self::onSetVol,
             state<Playing> + event<CmdSetVolume>                            / &Self::onSetVol,

             state<Ready>   + event<CmdPlay>                                 / &Self::onPlay      = state<Playing>,

             state<Playing> + event<CmdStop>                                 / &Self::onStop      = state<Ready>,
             state<Playing> + event<EvtPlaybackDone>                         / &Self::onPlaybackDone,
             state<Playing> + event<EvtGapEnd>                               / &Self::onGapEnd,

             state<Ready>   + event<CmdTeardown>                             / &Self::onTeardown  = state<Idle>,
             state<Playing> + event<CmdTeardown>                             / &Self::onTeardown  = state<Idle>,
             state<Error>   + event<CmdTeardown>                             / &Self::onTeardown  = state<Idle>
            // clang-format on
        );
    }

  private:
    /* Guards (no event parameters to avoid DI issues) */
    bool tryInitHal() {
        SS_LOGI("init HAL");
        return hal_.init();
    }

    /* Actions */
    void onSetupOk() { ctrlQ_.post(ctrl::EvtAudioReady{}); }
    void onSetupFail() { ctrlQ_.post(ctrl::EvtAudioError{}); }

    void onTeardown() {
        SS_LOGI("teardown");
        hal_.stop();
        timer_.stop();
    }

    void onStop() {
        SS_LOGI("stop");
        hal_.stop();
        timer_.stop();
        ctrlQ_.post(ctrl::EvtAudioDone{}); // user stop => done
    }

    void onSetVol(const CmdSetVolume &v) {
        SS_LOGI("volume=%hhu", v.volPct);
        hal_.setVolume(v.volPct);
    }

    // Start a new playlist (no guard; we validate here)
    void onPlay(const CmdPlay &e) {
        spec_     = e.spec;
        idx_      = 0;
        loopDone_ = 0;
        gapArmed_ = false;

        SS_LOGI("play: n=%hhu loops=%hu", spec_.n, spec_.playCnt);
        if (spec_.n == 0) {
            SS_LOGE("empty list");
            // treat as immediate completion from ctrl POV
            ctrlQ_.post(ctrl::EvtAudioError{});
            // Stop anything just in case
            hal_.stop();
            timer_.stop();
            // We’re technically in Playing due to the transition; next external
            // command (stop/teardown/new play) will leave it. This keeps FSM simple.
            return;
        }
        (void) startCurrent_();
    }

    void onPlaybackDone() {
        const bool lastInList = (idx_ + 1u >= spec_.n);
        if (!lastInList) {
            const uint16_t gap  = spec_.items[idx_].gapMs;
            const uint8_t  next = static_cast<uint8_t>(idx_ + 1u);
            if (gap > 0) {
                armGap_(gap, next);
            } else {
                idx_ = next;
                (void) startCurrent_();
            }
            return;
        }

        const uint16_t loops     = spec_.playCnt;
        const bool     infinite  = (loops == 0);
        const bool     moreLoops = infinite || (loopDone_ + 1u < loops);

        if (moreLoops) {
            const uint16_t gap  = spec_.items[idx_].gapMs; // gap after last before restarting
            const uint8_t  next = 0u;
            if (!infinite) ++loopDone_;
            if (gap > 0) {
                armGap_(gap, next);
            } else {
                idx_ = next;
                (void) startCurrent_();
            }
        } else {
            finishAll_();
        }
    }

    void onGapEnd() {
        gapArmed_ = false;
        idx_      = pendingNext_;
        (void) startCurrent_();
    }

    /* Helpers */
    bool startCurrent_() {
        if (idx_ >= spec_.n) return false;
        const char *src = spec_.items[idx_].source;
        SS_LOGI("start: i=%hhu src=%s", idx_, src);
        if (!src || *src == '\0') {
            SS_LOGE("bad source at %hhu", idx_);
            return false;
        }
        const bool ok = hal_.play(src);
        if (!ok) SS_LOGE("hal.play failed");
        return ok;
    }

    void armGap_(uint16_t ms, uint8_t nextIndex) {
        pendingNext_ = nextIndex;
        gapArmed_    = true;
        SS_LOGI("gap %hu ms -> next=%hhu", ms, nextIndex);
        // AO registered the timer callback (LED pattern). ITimer expects microseconds.
        timer_.startOnce(static_cast<uint64_t>(ms) * 1000ULL);
    }

    void finishAll_() {
        SS_LOGI("sequence complete");
        timer_.stop();
        ctrlQ_.post(ctrl::EvtAudioDone{});
        // (Stays in Playing until next command; mirrors LED style where completion notifies ctrl)
    }

  private:
    static constexpr char TAG[] = "AudioFSM";

    ctrl::Q        &ctrlQ_;
    hal::IAudioHAL &hal_;
    timer::ITimer  &timer_;

    AudioPlaySpec spec_{};
    uint8_t       idx_{0};
    uint16_t      loopDone_{0};
    uint8_t       pendingNext_{0};
    bool          gapArmed_{false};
};

} // namespace esphome::smart_signage::audio
