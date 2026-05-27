#include <riz/container/detail/lockfree/spsc_byte_ring_queue.h>

#include <algorithm>
#include <cstring>

using namespace riz::container::detail::lockfree;

std::size_t spsc_byte_ring_queue::push(const std::byte* data, std::size_t len) noexcept {
    if (len == 0) {
        return 0;
    }
    std::size_t tail = tail_.load(std::memory_order_relaxed);
    std::size_t head = head_.load(std::memory_order_acquire);
    std::size_t free = (head - tail - 1 + capacity_) % capacity_;
    std::size_t to_write = std::min(free, len);
    if (to_write == 0) {
        return 0;
    }
    std::size_t first = std::min(to_write, capacity_ - tail);
    std::memcpy(buff_ + tail, data, first);
    if (first < to_write) {
        std::memcpy(buff_, data + first, to_write - first);
    }
    tail_.store((tail + to_write) % capacity_, std::memory_order_release);
    return to_write;
}

std::size_t spsc_byte_ring_queue::pop_front(std::byte* data, std::size_t max_len) noexcept {
    if (max_len == 0) {
        return 0;
    }
    std::size_t tail = tail_.load(std::memory_order_acquire);
    std::size_t head = head_.load(std::memory_order_relaxed);
    std::size_t available = (tail - head + capacity_) % capacity_;
    std::size_t to_read = std::min(available, max_len);
    std::size_t first = std::min(to_read, capacity_ - head);
    std::memcpy(data, buff_ + head, first);
    if (first < to_read) {
        std::memcpy(data + first, buff_, to_read - first);
    }
    head_.store((head + to_read) % capacity_, std::memory_order_release);
    return to_read;
}

std::size_t spsc_byte_ring_queue::size() const noexcept {
    std::size_t tail = tail_.load(std::memory_order_acquire);
    std::size_t head = head_.load(std::memory_order_relaxed);
    return (tail - head + capacity_) % capacity_;
}
