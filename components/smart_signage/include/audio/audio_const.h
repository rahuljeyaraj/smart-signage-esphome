#pragma once
#include <cstddef>
#include <cstdint>

namespace esphome::smart_signage::audio {

constexpr size_t  kSourceStrLen  = 96;  // generic source string (file path / TTS key)
constexpr uint8_t kMaxPlaylist   = 8;   // entries per event
constexpr uint8_t kDefaultVolPct = 100; // keep your existing default if already set
constexpr uint8_t kMinVolPct     = 0;
constexpr uint8_t kMaxVolPct     = 100;

} // namespace esphome::smart_signage::audio
