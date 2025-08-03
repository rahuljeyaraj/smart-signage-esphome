#pragma once
#include "active_object.h"
#include "imu/imu_fsm.h"
#include "imu/imu_q.h"

namespace esphome::smart_signage::imu {

static constexpr char AO_TAG[] = "imuAo";

using AO = ActiveObject<Q, FSM, AO_TAG>;

} // namespace esphome::smart_signage::imu