#pragma once
#include <cstdint>
#include <cstring>
#include "audio/audio_const.h"

namespace esphome::smart_signage::audio {

// A single playlist entry with its post-item gap in ms
struct AudioPlayItem {
    char     source[kSourceStrLen]{}; // file (today) or TTS key (future)
    uint16_t gapMs{0};                // gap after this item finishes
};

struct AudioPlaySpec {
    uint8_t       n{0};       // number of valid items
    uint16_t      playCnt{1}; // 0 = infinite
    AudioPlayItem items[kMaxPlaylist];
};

} // namespace esphome::smart_signage::audio
