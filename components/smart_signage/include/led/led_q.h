#pragma once
#include "queue.h"
#include "led/led_event.h"

namespace esphome::smart_signage::led {
#ifndef LED_QUEUE_LEN
#define LED_QUEUE_LEN 8
#endif

using Q = Queue<Event, LED_QUEUE_LEN>;

} // namespace esphome::smart_signage::led