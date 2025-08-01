#include <etl/variant.h>
#include <freertos/FreeRTOS.h>

namespace esphome::smart_signage {

namespace radar {
struct Setup {};
struct Start {};
struct Stop {};
struct Teardown {};
struct TimerPoll {};
struct SetDistCm {
    uint16_t cm;
};
struct SetSampleInt {
    uint32_t ms;
};
using RxEvent = etl::variant<Setup, Start, Stop, Teardown, TimerPoll, SetDistCm, SetSampleInt>;

struct Error {};
struct Ready {};
struct Data {
    bool detected = false;
    uint16_t distanceCm = 0;
    TickType_t timestampTicks = 0;
};
using TxEvent = etl::variant<Error, Ready, Data>;
}  // namespace radar

namespace imu {
struct Setup {};
struct Start {};
struct Stop {};
struct Teardown {};
struct TimerPoll {};
using RxEvent = etl::variant<Setup, Start, Stop, Teardown, TimerPoll>;

struct Error {};
struct Ready {};
struct Fell {};
struct Rose {};
using TxEvent = etl::variant<Error, Ready, Fell, Rose>;
}  // namespace imu

namespace led {
struct Setup {};
struct Teardown {};
struct On {
    uint8_t brightPct = 100;
};
struct Off {};
struct Breathe {
    uint8_t brightPct = 100;
    uint16_t fadeInMs = 0;
    uint16_t fadeOutMs = 0;
    uint16_t maxCycles = 0;
};
struct FadeEnd {};
using RxEvent = etl::variant<Setup, Teardown, On, Off, Breathe, FadeEnd>;

struct Error {};
struct Ready {};
struct BreathDone {};
using TxEvent = etl::variant<Error, Ready, BreathDone>;
}  // namespace led

namespace audio {
static constexpr size_t kLen = 64;
struct Setup {};
struct Teardown {};
struct Play {
    char filePath[kLen];
    uint8_t volPct;
};
struct Stop {};
using RxEvent = etl::variant<Setup, Teardown, Play, Stop>;

struct Error {};
struct Ready {};
struct PlayDone {};
using TxEvent = etl::variant<Error, Ready, PlayDone>;
}  // namespace audio

namespace ctrl {
struct Setup {};
struct Start {
    uint32_t runTimeMins;
};
struct Stop {};
struct Timeout {};

using Ready = etl::variant<radar::Ready, imu::Ready, led::Ready, audio::Ready>;
using Error = etl::variant<radar::Error, imu::Error, led::Error, audio::Error>;
using RxEvent = etl::variant<Setup, Start, Ready, Error, Timeout, radar::TxEvent, imu::TxEvent,
                             led::TxEvent, audio::TxEvent>;
}  // namespace ctrl
}  // namespace esphome::smart_signage