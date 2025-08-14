#pragma once
#include <etl/string.h>

#ifndef SS_MAX_PROFILES
#define SS_MAX_PROFILES 8
#endif

#ifndef SS_MAX_EVENTS_TOTAL
#define SS_MAX_EVENTS_TOTAL 32
#endif

namespace esphome::smart_signage {

using ProfileName = etl::string<15>;
using ProfileList = etl::array<ProfileName, SS_MAX_PROFILES>;

} // namespace esphome::smart_signage