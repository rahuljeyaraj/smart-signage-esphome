#pragma once
#include <etl/variant.h>
#include <etl/algorithm.h>
#include "audio/audio_const.h"
#include "audio/audio_play_spec.h"

namespace esphome::smart_signage::audio {

// ── Commands ────────────────────────────────────────────────────────────────
struct CmdSetup {};
struct CmdTeardown {};
struct CmdStop {};

struct SetVolume {
    uint8_t volPct;
    explicit SetVolume(uint8_t v = kDefaultVolPct)
        : volPct(etl::clamp<uint8_t>(v, kMinVolPct, kMaxVolPct)) {}
};

struct CmdPlay {
    AudioPlaySpec spec;
};

// ── Internal events (AO posts → FSM consumes) ───────────────────────────────
struct EvtPlaybackDone {}; // HAL finished current item (natural EOF)
struct EvtGapEnd {};       // AO timer expired

using Event =
    etl::variant<CmdSetup, CmdTeardown, CmdStop, SetVolume, CmdPlay, EvtPlaybackDone, EvtGapEnd>;

} // namespace esphome::smart_signage::audio
