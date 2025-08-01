// #pragma once
// #include <etl/variant.h>
// #include <freertos/FreeRTOS.h>

// namespace esphome::smart_signage {

// namespace imu {
// struct Setup {};
// struct Start {};
// struct Stop {};
// struct Teardown {};
// struct TimerPoll {};
// using RxEvent = etl::variant<Setup, Start, Stop, Teardown, TimerPoll>;

// struct InitError {};
// struct SetupDone {};
// struct Fell {};
// struct Rose {};
// using TxEvent = etl::variant<InitError, SetupDone, Fell, Rose>;
// } // namespace imu

// namespace led {
// struct Setup {};
// struct Teardown {};
// struct On {
//     uint8_t brightPct = 100;
// };
// struct Off {};
// struct Breathe {
//     uint8_t brightPct = 100;
//     uint16_t fadeInMs = 0;
//     uint16_t fadeOutMs = 0;
//     uint16_t maxCycles = 0;
// };
// struct FadeEnd {};
// using RxEvent = etl::variant<Setup, Teardown, On, Off, Breathe, FadeEnd>;

// struct InitError {};
// struct SetupDone {};
// struct BreathDone {};
// using TxEvent = etl::variant<InitError, SetupDone, BreathDone>;
// } // namespace led

// namespace audio {
// static constexpr size_t kLen = 64;
// struct Setup {};
// struct Teardown {};
// struct Play {
//     char filePath[kLen];
//     uint8_t volPct;
// };
// struct Stop {};
// using RxEvent = etl::variant<Setup, Teardown, Play, Stop>;

// struct InitError {};
// struct SetupDone {};
// struct PlayDone {};
// using TxEvent = etl::variant<InitError, SetupDone, PlayDone>;
// } // namespace audio

// namespace ctrl {
// struct Setup {};
// struct Start {
//     uint32_t runTimeMins;
// };
// struct Stop {};
// struct Timeout {};

// using SetupDone = etl::variant<imu::SetupDone, led::SetupDone, audio::SetupDone>;
// using InitError = etl::variant<radar::InitError, imu::InitError, led::InitError,
// audio::InitError>; using RxEvent = etl::variant<Setup, Start, radar::SetupDone, SetupDone,
// InitError, Timeout,
//                              radar::TxEvent, imu::TxEvent, led::TxEvent, audio::TxEvent>;
// } // namespace ctrl
// } // namespace esphome::smart_signage