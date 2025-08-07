#pragma once
#include "active_object.h"
#include "audio/audio_fsm.h"
#include "audio/audio_q.h"

namespace esphome::smart_signage::audio {

using AO = ActiveObject<Q, FSM>;

} // namespace esphome::smart_signage::audio