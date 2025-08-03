#pragma once
#include "queue.h"
#include "audio/audio_event.h"

namespace esphome::smart_signage::audio {
#ifndef AUDIO_QUEUE_LEN
#define AUDIO_QUEUE_LEN 8
#endif

using Q = Queue<Event, AUDIO_QUEUE_LEN>;

} // namespace esphome::smart_signage::audio