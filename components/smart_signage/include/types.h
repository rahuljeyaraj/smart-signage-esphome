#pragma once
#include <etl/string.h>

namespace esphome::smart_signage {
// ────────────────────────────────────────────────────────────────────────────
// Fixed-width profile name type (ASCII). Max 15 chars (+ '\0' internally).
// Use this everywhere instead of indices.
// ────────────────────────────────────────────────────────────────────────────
using ProfileName = etl::string<15>;
}