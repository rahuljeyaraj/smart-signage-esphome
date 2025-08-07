#pragma once
#include "active_object.h"
#include "imu/imu_fsm.h"
#include "imu/imu_q.h"

namespace esphome::smart_signage::imu {

using AO = ActiveObject<Q, FSM>;

} // namespace esphome::smart_signage::imu