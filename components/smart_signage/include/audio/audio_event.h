#pragma once
#include <etl/variant.h>
#include <etl/algorithm.h>
#include "audio/audio_const.h"

namespace esphome::smart_signage::audio {

// ── Commands ────────────────────────────────────────────────────────────────
struct CmdSetup {};
struct CmdTeardown {};
struct CmdStop {};

struct CmdSetVolume {
    uint8_t volPct;
    explicit CmdSetVolume(uint8_t v = kDefaultVolPct)
        : volPct(etl::clamp<uint8_t>(v, kMinVolPct, kMaxVolPct)) {}
};

// A single playlist entry with its post-item gap in ms
struct AudioPlayItem {
    char     source[kSourceStrLen]{}; // file (today) or TTS key (future)
    uint16_t gapMs{0};                // gap after this item finishes
};

struct AudioPlaySpec {
    uint8_t       n{0};         // number of valid items
    uint16_t      loopCount{1}; // 0 = infinite
    AudioPlayItem items[kMaxPlaylist];
};

struct CmdPlay {
    AudioPlaySpec spec;
};

// ── Internal events (AO posts → FSM consumes) ───────────────────────────────
struct EvtPlaybackDone {}; // HAL finished current item (natural EOF)
struct EvtGapEnd {};       // AO timer expired

using Event =
    etl::variant<CmdSetup, CmdTeardown, CmdStop, CmdSetVolume, CmdPlay, EvtPlaybackDone, EvtGapEnd>;

} // namespace esphome::smart_signage::audio
