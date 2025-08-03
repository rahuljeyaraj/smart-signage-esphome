#pragma once
#include "active_object.h"
#include "led/led_fsm.h"
#include "led/led_q.h"

namespace esphome::smart_signage::led {

static constexpr char AO_TAG[] = "ledAo";

using AO = ActiveObject<Q, FSM, AO_TAG>;

} // namespace esphome::smart_signage::led