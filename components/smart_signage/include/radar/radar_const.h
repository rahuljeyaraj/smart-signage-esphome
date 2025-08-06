#pragma once
#include <cstdint>

namespace esphome::smart_signage::radar {

constexpr uint32_t kDefaultRangeCm = 600;

constexpr uint32_t kMinSampleIntMs     = 20;
constexpr uint32_t kDefaultSampleIntMs = 200;

} // namespace esphome::smart_signage::radar
