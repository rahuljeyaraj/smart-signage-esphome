#pragma once
#include <cstddef>
#include <cstdint>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace esphome::smart_signage {

class Queue {
  public:
    // Constructor: specify event size (bytes) and length
    Queue(size_t queueLen, size_t eventSize) : eventSize_(eventSize) {
        queue_ = xQueueCreate(queueLen, eventSize_);
    }

    ~Queue() {
        if (queue_)
            vQueueDelete(queue_);
    }

    // Non-copyable
    Queue(const Queue &) = delete;
    Queue &operator=(const Queue &) = delete;

    // Movable (optional)
    Queue(Queue &&other) noexcept : queue_(other.queue_), eventSize_(other.eventSize_) {
        other.queue_ = nullptr;
    }
    Queue &operator=(Queue &&other) noexcept {
        if (this != &other) {
            queue_ = other.queue_;
            eventSize_ = other.eventSize_;
            other.queue_ = nullptr;
        }
        return *this;
    }

    // Type-erased post: pass pointer to event (copied)
    bool post(const void *event, TickType_t wait = 0) const {
        if (queue_ == nullptr)
            return false;
        return xQueueSend(queue_, event, wait) == pdPASS;
    }

    // Get the queue handle for raw API use
    QueueHandle_t handle() const { return queue_; }

    // Size in bytes of one event
    size_t event_size() const { return eventSize_; }

  private:
    QueueHandle_t queue_{nullptr};
    size_t eventSize_;
};

extern Queue radarQ;
extern Queue ctrlQ;

} // namespace esphome::smart_signage