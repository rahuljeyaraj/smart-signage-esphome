#pragma once

#include <cstddef>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

namespace esphome::smart_signage {

/**
 * @brief A fixed‐length FreeRTOS queue for any data type.
 *
 * @tparam ItemT       The type of object to store.
 * @tparam QueueLength Number of slots in the queue.
 */
template <typename ItemT, std::size_t QueueLength> class Queue {
    static_assert(QueueLength > 0, "QueueLength must be > 0");

  public:
    /// The type of element stored in the queue.
    using ItemType = ItemT;
    /// Alias matching classic container conventions.
    using size_type = std::size_t;

    /// Create the underlying FreeRTOS queue.
    Queue() { handle_ = xQueueCreate(QueueLength, sizeof(ItemType)); }

    /// Delete the underlying FreeRTOS queue (if any).
    ~Queue() {
        if (handle_) { vQueueDelete(handle_); }
    }

    // Non‐copyable
    Queue(const Queue &)            = delete;
    Queue &operator=(const Queue &) = delete;

    // Movable
    Queue(Queue &&o) noexcept : handle_(o.handle_) { o.handle_ = nullptr; }
    Queue &operator=(Queue &&o) noexcept {
        if (this != &o) {
            if (handle_) { vQueueDelete(handle_); }
            handle_   = o.handle_;
            o.handle_ = nullptr;
        }
        return *this;
    }

    /**
     * @brief Enqueue one element (copied).
     * @param item The element to enqueue.
     * @param wait How long to block if the queue is full (in ticks).
     * @return true on success (pdPASS), false otherwise.
     */
    bool post(const ItemType &item, TickType_t wait = 0) const {
        return handle_ && (xQueueSend(handle_, &item, wait) == pdPASS);
    }

    /**
     * @brief Peek at the next element without removing it.
     * @param out  Reference to store the peeked element.
     * @param wait How long to block if the queue is empty (in ticks).
     * @return true on success (pdPASS), false otherwise.
     */
    bool peek(ItemType &out, TickType_t wait = 0) const {
        return handle_ && (xQueuePeek(handle_, &out, wait) == pdPASS);
    }

    /// @return The raw FreeRTOS queue handle.
    QueueHandle_t handle() const { return handle_; }

    /// @return The queue capacity (runtime).
    size_type capacity() const { return QueueLength; }

    /// @brief Compile‐time constants.
    static constexpr size_type queue_length = QueueLength;
    static constexpr size_type item_size    = sizeof(ItemType);

  private:
    QueueHandle_t handle_{nullptr};
};

} // namespace esphome::smart_signage
