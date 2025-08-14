#pragma once
#include <cstdint>
#include <cstring>

namespace esphome::smart_signage::led {

enum class LedPattern : uint8_t { Unknown, Blink, Twinkle };

inline const char *ledPatternToCStr(LedPattern p) {
    switch (p) {
    case LedPattern::Blink: return "blink";
    case LedPattern::Twinkle: return "twinkle";
    default: return "unknown";
    }
}
inline LedPattern ledPatternFromCStr(const char *s) {
    if (!s || s[0] == '\0') return LedPattern::Unknown;
    if (std::strcmp(s, "blink") == 0) return LedPattern::Blink;
    if (std::strcmp(s, "twinkle") == 0) return LedPattern::Twinkle;
    return LedPattern::Unknown;
}

struct LedPatternSpec {
    LedPattern pattern{LedPattern::Unknown};
    uint16_t   periodMs{0}; // blink period
    uint16_t   cnt{0};      // 0 = infinite
};

} // namespace esphome::smart_signage::led