#include "ctrl/event.h"
#include "queue.h"

namespace esphome::smart_signage {
namespace ctrl {

#ifndef RADAR_QUEUE_LEN
#define RADAR_QUEUE_LEN 8
#endif

using Q = Queue<Event, RADAR_QUEUE_LEN>;

} // namespace ctrl
} // namespace esphome::smart_signage