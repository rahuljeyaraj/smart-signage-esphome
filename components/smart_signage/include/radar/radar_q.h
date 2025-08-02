#pragma once
#include "queue.h"
#include "radar/radar_event.h"

namespace esphome::smart_signage::radar {
#ifndef RADAR_QUEUE_LEN
#define RADAR_QUEUE_LEN 8
#endif

using Q = Queue<Event, RADAR_QUEUE_LEN>;

} // namespace esphome::smart_signage::radar