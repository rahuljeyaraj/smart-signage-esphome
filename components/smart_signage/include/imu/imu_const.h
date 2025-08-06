#pragma once
#include <cstdint>

namespace esphome::smart_signage::imu {

constexpr uint16_t kMaxFallAngleDeg     = 180;
constexpr uint16_t kDefaultFallAngleDeg = 45;

constexpr uint16_t kMinConfirmCount     = 1;
constexpr uint16_t kDefaultConfirmCount = 5;

constexpr uint32_t kMinSampleIntMs     = 20;
constexpr uint32_t kDefaultSampleIntMs = 200;

} // namespace esphome::smart_signage::imu
