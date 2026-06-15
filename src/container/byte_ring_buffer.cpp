#include <riz/container/byte_ring_buffer.h>

#include <algorithm>
#include <cstring>

using namespace riz::container;

std::size_t byte_ring_buffer::push(const std::byte* data, std::size_t len) noexcept {
    if (len == 0) {
        return 0;
    }
    std::size_t available = (head_ - tail_ - 1 + capacity_) % capacity_;
    std::size_t to_write = std::min(available, len);
    if (to_write == 0) {
        return 0;
    }
    std::size_t first = std::min(to_write, capacity_ - tail_);
    std::memcpy(buff_ + tail_, data, first);
    if (first < to_write) {
        std::memcpy(buff_, data + first, to_write - first);
    }
    tail_ = (tail_ + to_write) % capacity_;
    return to_write;
}

std::size_t byte_ring_buffer::pop_front(std::byte* data, std::size_t max_len) noexcept {
    if (max_len == 0) {
        return 0;
    }
    std::size_t available = (tail_ - head_ + capacity_) % capacity_;
    std::size_t to_read = std::min(available, max_len);
    std::size_t first = std::min(to_read, capacity_ - head_);
    std::memcpy(data, buff_ + head_, first);
    if (first < to_read) {
        std::memcpy(data + first, buff_, to_read - first);
    }
    head_ = (head_ + to_read) % capacity_;
    return to_read;
}

std::size_t byte_ring_buffer::size() const noexcept {
    return (tail_ - head_ + capacity_) % capacity_;
}
