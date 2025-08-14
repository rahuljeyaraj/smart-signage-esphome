#pragma once
#include <etl/string.h>
#include <etl/vector.h>
#include "radar/radar_const.h"
#include "audio/audio_const.h"
#include "led/led_const.h"
#include "ctrl/ctrl_const.h"

#ifndef SS_MAX_PROFILES
#define SS_MAX_PROFILES 8
#endif

#ifndef SS_MAX_EVENTS_TOTAL
#define SS_MAX_EVENTS_TOTAL 32
#endif

namespace esphome::smart_signage {

using ProfileName = etl::string<15>;

using ProfileNames = etl::vector<ProfileName, SS_MAX_PROFILES>;

struct ProfileValues {
    uint32_t sessionMins{ctrl::kDefaultSessionMins};
    uint32_t radarRangeCm{radar::kDefaultRangeCm};
    uint8_t  audioVolPct{audio::kDefaultVolPct};
    uint8_t  ledBrightPct{led::kDefaultBrightPct};
};

} // namespace esphome::smart_signage