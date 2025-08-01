#pragma once

#include <cstddef>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace esphome::smart_signage {

/**
 * A fixed‐length FreeRTOS queue for exactly one Event type.
 *
 * @tparam Event     The type of object you’ll store (e.g. radar::Event)
 * @tparam QueueLen  Number of slots in the queue
 */
template <typename Event, size_t QueueLen> class Queue {
  public:
    Queue() { queue_ = xQueueCreate(QueueLen, sizeof(Event)); }
    ~Queue() {
        if (queue_) {
            vQueueDelete(queue_);
        }
    }

    // Non-copyable
    Queue(const Queue &) = delete;
    Queue &operator=(const Queue &) = delete;

    // Movable
    Queue(Queue &&o) noexcept : queue_(o.queue_) { o.queue_ = nullptr; }
    Queue &operator=(Queue &&o) noexcept {
        if (this != &o) {
            queue_ = o.queue_;
            o.queue_ = nullptr;
        }
        return *this;
    }

    /** Enqueue one Event (copied). */
    bool post(const Event &e, TickType_t wait = 0) const {
        return queue_ && xQueueSend(queue_, &e, wait) == pdPASS;
    }

    /** Access the raw FreeRTOS handle. */
    QueueHandle_t handle() const { return queue_; }

    /** Compile-time constants: */
    static constexpr size_t event_size = sizeof(Event);
    static constexpr size_t queue_length = QueueLen;

  private:
    QueueHandle_t queue_{nullptr};
};

} // namespace esphome::smart_signage
