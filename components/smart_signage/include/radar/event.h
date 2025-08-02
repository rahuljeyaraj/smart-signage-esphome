#pragma once

#include <etl/variant.h>

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
using Event = etl::variant<Setup, Start, Stop, Teardown, TimerPoll, SetDistCm, SetSampleInt>;

// struct InitError {};
// struct SetupDone {};
// struct Data {
//     bool detected = false;
//     uint16_t distanceCm = 0;
//     TickType_t timestampTicks = 0;
// };
// using TxEvent = etl::variant<InitError, SetupDone, Data>;

} // namespace radar
} // namespace esphome::smart_signage