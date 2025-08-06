#pragma once
#include <etl/variant.h>
#include <etl/algorithm.h>
#include "audio/audio_const.h"

namespace esphome::smart_signage::audio {

struct CmdSetup {};
struct CmdTeardown {};
struct CmdStop {};
struct CmdPlay {
    char    filePath[kFilePathStrLen]{};
    uint8_t volPct = kDefaultVolPct;

    CmdPlay(const char *path, uint8_t v = kDefaultVolPct)
        : volPct(etl::clamp(v, kMinVolPct, kMaxVolPct)) {
        if (path) { snprintf(filePath, sizeof(filePath), "%s", path); }
    }
};

using Event = etl::variant<CmdSetup, CmdTeardown, CmdStop, CmdPlay>;

} // namespace esphome::smart_signage::audio
