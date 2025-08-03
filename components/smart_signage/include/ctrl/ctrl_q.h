#pragma once
#include "ctrl/ctrl_event.h"
#include "queue.h"

namespace esphome::smart_signage::ctrl {

#ifndef CTRL_QUEUE_LEN
#define CTRL_QUEUE_LEN 8
#endif

using Q = Queue<Event, CTRL_QUEUE_LEN>;

} // namespace esphome::smart_signage::ctrl