#include "queue.h"
#include "radar/event.h"

namespace esphome::smart_signage {
namespace radar {

#ifndef RADAR_QUEUE_DEPTH
#define RADAR_QUEUE_DEPTH 8
#endif

using Q = Queue<Event, RADAR_QUEUE_DEPTH>;

} // namespace radar
} // namespace esphome::smart_signage