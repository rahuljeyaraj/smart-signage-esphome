#pragma once
#include "active_object.h"
#include "led/led_fsm.h"
#include "led/led_q.h"

namespace esphome::smart_signage::led {

using AO = ActiveObject<Q, FSM>;

} // namespace esphome::smart_signage::led