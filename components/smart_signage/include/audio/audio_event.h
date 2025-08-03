#pragma once
#include <etl/variant.h>
#include <etl/algorithm.h> // etl::clamp

namespace esphome::smart_signage::audio {

/*────────────────────────── Limits & defaults ──────────────────────────*/
constexpr uint16_t kMinVolPct     = 0;
constexpr uint16_t kMaxVolPct     = 100;
constexpr uint16_t kDefaultVolPct = 100;

constexpr size_t kFilePathStrLen = 64;

/*────────────────────────── Commands (Ctrl → AUDIO) ──────────────────────*/
struct CmdSetup {};
struct CmdTeardown {};
struct CmdPlay {
    char    filePath[kFilePathStrLen]{};
    uint8_t volPct = kDefaultVolPct;

    CmdPlay(const char *path, uint16_t v = kDefaultVolPct)
        : volPct(static_cast<uint8_t>(etl::clamp(v, kMinVolPct, kMaxVolPct))) {
        if (path) { snprintf(filePath, sizeof(filePath), "%s", path); }
    }
};
struct CmdStop {};

/*────────────────────────── Unified variant type ───────────────────────*/
using Event = etl::variant<
    /* commands */
    CmdSetup,
    CmdTeardown,
    CmdPlay,
    CmdStop>;

} // namespace esphome::smart_signage::audio
