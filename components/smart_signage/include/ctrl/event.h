#pragma once

#include <etl/variant.h>

namespace esphome::smart_signage::ctrl {

namespace radar {
    struct InitError {};
    struct SetupDone {};
    struct Data {
        bool       detected       = false;
        uint16_t   distanceCm     = 0;
        TickType_t timestampTicks = 0;
    };
} // namespace radar

namespace imu {
    struct InitError {};
    struct SetupDone {};
    struct Fell {};
    struct Rose {};
} // namespace imu

namespace led {
    struct InitError {};
    struct SetupDone {};
    struct BreathDone {};
} // namespace led

namespace audio {
    struct InitError {};
    struct SetupDone {};
    struct PlayDone {};
} // namespace audio

struct Setup {};
struct Start {
    uint32_t runTimeMins;
};
struct Stop {};
struct Timeout {};

using Event = etl::variant<Setup,
                           Start,
                           Stop,
                           Timeout,
                           radar::InitError,
                           radar::SetupDone,
                           radar::Data,
                           imu::InitError,
                           imu::SetupDone,
                           imu::Fell,
                           imu::Rose,
                           led::InitError,
                           led::SetupDone,
                           led::BreathDone,
                           audio::InitError,
                           audio::SetupDone,
                           audio::PlayDone>;

} // namespace esphome::smart_signage::ctrl