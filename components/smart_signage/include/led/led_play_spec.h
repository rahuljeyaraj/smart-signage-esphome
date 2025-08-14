#pragma once
#include <cstdint>
#include <cstring>

namespace esphome::smart_signage::led {

enum class LedPattern : uint8_t { Unknown, Square, Triangle };

inline const char *ledPatternToCStr(LedPattern p) {
    switch (p) {
    case LedPattern::Square: return "square";
    case LedPattern::Triangle: return "triangle";
    default: return "unknown";
    }
}
inline LedPattern ledPatternFromCStr(const char *s) {
    if (!s || s[0] == '\0') return LedPattern::Unknown;
    if (std::strcmp(s, "square") == 0) return LedPattern::Square;
    if (std::strcmp(s, "triangle") == 0) return LedPattern::Triangle;
    return LedPattern::Unknown;
}

struct LedPlaySpec {
    LedPattern pattern{LedPattern::Unknown};
    uint16_t   periodMs{0}; // blink period
    uint16_t   cnt{0};      // 0 = infinite
};

} // namespace esphome::smart_signage::led