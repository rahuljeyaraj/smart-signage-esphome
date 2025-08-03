#pragma once
#include "active_object.h"
#include "audio/audio_fsm.h"
#include "audio/audio_q.h"

namespace esphome::smart_signage::audio {

static constexpr char AO_TAG[] = "audioAo";

using AO = ActiveObject<Q, FSM, AO_TAG>;

} // namespace esphome::smart_signage::audio