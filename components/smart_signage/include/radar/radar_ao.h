#pragma once
#include "active_object.h"
#include "radar/radar_fsm.h"
#include "radar/radar_q.h"

namespace esphome::smart_signage::radar {

static constexpr char AO_TAG[] = "radarAo";

using AO = ActiveObject<Q, FSM, AO_TAG>;

} // namespace esphome::smart_signage::radar