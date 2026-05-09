#include <riz/container/detail/ring_queue.h>

#include <cassert>
#include <cstring>
#include <new>
#include <utility>

using namespace riz::container::detail;

ring_queue::ring_queue(
    void* buffer, std::size_t entry_size, std::size_t capacity)
    : buffer_ {static_cast<std::byte*>(buffer)}
    , entry_size_ {entry_size}
    , capacity_ {capacity} {}

ring_queue::ring_queue(ring_queue&& rhs) noexcept
    : buffer_ {std::exchange(rhs.buffer_, {})}
    , entry_size_ {std::exchange(rhs.entry_size_, 0)}
    , capacity_ {std::exchange(rhs.capacity_, 0)}
    , head_ {std::exchange(rhs.head_, 0)}
    , tail_ {std::exchange(rhs.tail_, 0)}
    , size_ {std::exchange(rhs.size_, 0)} {}

ring_queue& ring_queue::operator=(ring_queue&& rhs) noexcept {
    if (this != &rhs) {
        this->~ring_queue();
        new(this) ring_queue {std::move(rhs)};
    }
    return *this;
}

bool ring_queue::pop_front(void* value) noexcept {
    assert(value);
    if (empty()) {
        return false;
    }
    std::memcpy(value, buffer_ + head_ * entry_size_, entry_size_);
    head_ = (head_ + 1) % capacity_;
    --size_;
    return true;
}

void ring_queue::push(const void* value) noexcept {
    assert(value);
    const bool full = (size_ == capacity_);
    std::memcpy(buffer_ + tail_ * entry_size_, value, entry_size_);
    tail_ = (tail_ + 1) % capacity_;
    if (!full) {
        ++size_;
    } else {
        head_ = (head_ + 1) % capacity_;
    }
}
