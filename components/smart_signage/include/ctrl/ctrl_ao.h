#pragma once
#include "active_object.h"
#include "ctrl/ctrl_fsm.h"
#include "ctrl/ctrl_q.h"

namespace esphome::smart_signage::ctrl {

static constexpr char AO_TAG[] = "ctrlAo";

using AO = ActiveObject<Q, FSM, AO_TAG>;

} // namespace esphome::smart_signage::ctrl