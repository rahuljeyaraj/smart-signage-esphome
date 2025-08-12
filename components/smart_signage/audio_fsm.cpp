// #include "audio/audio_fsm.h"
// #include "ctrl/ctrl_event.h"
// #include "log.h"

// namespace esphome::smart_signage::audio {

// static constexpr char TAG[] = "AudioFSM";

// FSM::FSM(ctrl::Q &ctrlQ, hal::IAudioHAL &hal, timer::ITimer &timer)
//     : ctrlQ_(ctrlQ), hal_(hal), timer_(timer) {}

// auto FSM::operator()() noexcept {
//     struct Idle {};
//     struct Ready {};
//     struct Playing {};
//     struct Error {};

//     using namespace sml;
//     return make_transition_table(
//         // clang-format off
//         *state<Idle>    + event<CmdSetup>       [ &FSM::tryInitHal ] / &FSM::onSetupOk   =
//         state<Ready>,
//          state<Idle>    + event<CmdSetup>                          / &FSM::onSetupFail =
//          state<Error>,

//          state<Ready>   + event<CmdSetVolume>                      / &FSM::onSetVol,
//          state<Playing> + event<CmdSetVolume>                      / &FSM::onSetVol,

//          state<Ready>   + event<CmdPlay>        [ &FSM::tryPlay ]                   =
//          state<Playing>, state<Ready>   + event<CmdPlay>                          /
//          &FSM::onSetupFail = state<Error>,

//          state<Playing> + event<CmdStop>                          / &FSM::onStop     =
//          state<Ready>, state<Playing> + event<EvtPlaybackDone>                  /
//          &FSM::onPlaybackDone, state<Playing> + event<EvtGapEnd>                        /
//          &FSM::onGapEnd,

//          state<Ready>   + event<CmdTeardown>                      / &FSM::onTeardown =
//          state<Idle>, state<Playing> + event<CmdTeardown>                      / &FSM::onTeardown
//          = state<Idle>, state<Error>   + event<CmdTeardown>                      /
//          &FSM::onTeardown = state<Idle>,

//          state<Error>   + sml::on_entry<_>                        / ([](auto const&){
//          SS_LOGE("ERROR"); })
//         // clang-format on
//     );
// }

// /* Guards */
// bool FSM::tryInitHal(const CmdSetup &) {
//     SS_LOGI("init HAL");
//     return hal_.init();
// }

// bool FSM::tryPlay(const CmdPlay &e) {
//     spec_     = e.spec;
//     idx_      = 0;
//     loopDone_ = 0;
//     gapArmed_ = false;

//     SS_LOGI("play: n=%hhu loops=%hu", spec_.n, spec_.loopCount);
//     if (spec_.n == 0) {
//         SS_LOGE("empty list");
//         return false;
//     }
//     return startCurrent_();
// }

// /* Actions */
// void FSM::onSetupOk(const CmdSetup &) { ctrlQ_.post(ctrl::EvtAudioReady{}); }
// void FSM::onSetupFail(const CmdSetup &) { ctrlQ_.post(ctrl::EvtAudioError{}); }

// void FSM::onTeardown(const CmdTeardown &) {
//     SS_LOGI("teardown");
//     hal_.stop();
//     timer_.stop();
// }

// void FSM::onStop(const CmdStop &) {
//     SS_LOGI("stop");
//     hal_.stop();
//     timer_.stop();
//     ctrlQ_.post(ctrl::EvtAudioDone{}); // user stop => done from ctrl POV
// }

// void FSM::onSetVol(const CmdSetVolume &v) {
//     SS_LOGI("volume=%hhu", v.volPct);
//     hal_.setVolume(v.volPct);
// }

// void FSM::onPlaybackDone(const EvtPlaybackDone &) {
//     const bool lastInList = (idx_ + 1u >= spec_.n);
//     if (!lastInList) {
//         const uint16_t gap  = spec_.items[idx_].gapMs;
//         const uint8_t  next = static_cast<uint8_t>(idx_ + 1u);
//         if (gap > 0) {
//             armGap_(gap, next);
//         } else {
//             idx_ = next;
//             (void) startCurrent_();
//         }
//         return;
//     }

//     const uint16_t loops     = spec_.loopCount;
//     const bool     infinite  = (loops == 0);
//     const bool     moreLoops = infinite || (loopDone_ + 1u < loops);

//     if (moreLoops) {
//         const uint16_t gap  = spec_.items[idx_].gapMs; // gap after last item before restarting
//         const uint8_t  next = 0u;
//         if (!infinite) ++loopDone_;
//         if (gap > 0) {
//             armGap_(gap, next);
//         } else {
//             idx_ = next;
//             (void) startCurrent_();
//         }
//     } else {
//         finishAll_();
//     }
// }

// void FSM::onGapEnd(const EvtGapEnd &) {
//     gapArmed_ = false;
//     idx_      = pendingNext_;
//     (void) startCurrent_();
// }

// /* Helpers */
// bool FSM::startCurrent_() {
//     if (idx_ >= spec_.n) return false;
//     const char *src = spec_.items[idx_].source;
//     SS_LOGI("start: i=%hhu src=%s", idx_, src);
//     if (!src || *src == '\0') {
//         SS_LOGE("bad source at %hhu", idx_);
//         return false;
//     }
//     const bool ok = hal_.play(src);
//     if (!ok) SS_LOGE("hal.play failed");
//     return ok;
// }

// void FSM::armGap_(uint16_t ms, uint8_t nextIndex) {
//     pendingNext_ = nextIndex;
//     gapArmed_    = true;
//     SS_LOGI("gap %hu ms -> next=%hhu", ms, nextIndex);
//     // ITimer takes microseconds; AO registered the callback already.
//     timer_.startOnce(static_cast<uint64_t>(ms) * 1000ULL);
// }

// void FSM::finishAll_() {
//     SS_LOGI("sequence complete");
//     timer_.stop();
//     ctrlQ_.post(ctrl::EvtAudioDone{});
// }

// } // namespace esphome::smart_signage::audio
