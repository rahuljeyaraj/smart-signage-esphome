#pragma once
#include <cstdint>

namespace esphome::smart_signage::radar {

constexpr uint32_t kDefaultRangeCm = 600;

constexpr uint32_t   kMinSampleIntMs     = 20;
constexpr uint32_t   kDefaultSampleIntMs = 100;
static constexpr int kMaxFlushCnt        = 20;

constexpr float kProcessNoise     = 0.01f;
constexpr float kMeasurementNoise = 2.0f;
constexpr float kInitialError     = 5.0f;

} // namespace esphome::smart_signage::radar
