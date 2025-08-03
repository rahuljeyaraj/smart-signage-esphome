#pragma once
#include "queue.h"
#include "imu/imu_event.h"

namespace esphome::smart_signage::imu {
#ifndef IMU_QUEUE_LEN
#define IMU_QUEUE_LEN 8
#endif

using Q = Queue<Event, IMU_QUEUE_LEN>;

} // namespace esphome::smart_signage::imu