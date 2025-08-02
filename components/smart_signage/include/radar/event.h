#pragma once

#include <etl/variant.h>

namespace esphome::smart_signage::radar {

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

} // namespace esphome::smart_signage::radar