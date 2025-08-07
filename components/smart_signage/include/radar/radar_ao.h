#pragma once
#include "active_object.h"
#include "radar/radar_fsm.h"
#include "radar/radar_q.h"

namespace esphome::smart_signage::radar {

using AO = ActiveObject<Q, FSM>;

} // namespace esphome::smart_signage::radar