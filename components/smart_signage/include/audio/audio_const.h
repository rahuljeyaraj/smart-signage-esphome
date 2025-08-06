#pragma once
#include <cstdint>

namespace esphome::smart_signage::audio {

constexpr uint8_t kMinVolPct      = 0;
constexpr uint8_t kMaxVolPct      = 100;
constexpr uint8_t kDefaultVolPct  = 100;
constexpr size_t  kFilePathStrLen = 64;

} // namespace esphome::smart_signage::audio
