#include <riz/container/raw_ring_queue.h>

#include <cassert>
#include <cstring>
#include <new>
#include <utility>

using namespace riz::container;

raw_ring_queue::raw_ring_queue(void* buffer, std::size_t entry_size, std::size_t capacity)
    : buffer_ {static_cast<std::byte*>(buffer)}
    , entry_size_ {entry_size}
    , capacity_ {capacity} {}

raw_ring_queue::raw_ring_queue(raw_ring_queue&& rhs) noexcept
    : buffer_ {std::exchange(rhs.buffer_, {})}
    , entry_size_ {std::exchange(rhs.entry_size_, 0)}
    , capacity_ {std::exchange(rhs.capacity_, 0)}
    , head_ {std::exchange(rhs.head_, 0)}
    , tail_ {std::exchange(rhs.tail_, 0)}
    , size_ {std::exchange(rhs.size_, 0)} {}

raw_ring_queue& raw_ring_queue::operator=(raw_ring_queue&& rhs) noexcept {
    if (this != &rhs) {
        this->~raw_ring_queue();
        new (this) raw_ring_queue {std::move(rhs)};
    }
    return *this;
}

bool raw_ring_queue::pop_front(void* value) noexcept {
    assert(value);
    if (empty()) {
        return false;
    }
    std::memcpy(value, buffer_ + head_ * entry_size_, entry_size_);
    head_ = (head_ + 1) % capacity_;
    --size_;
    return true;
}

void raw_ring_queue::push(const void* value) noexcept {
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

void raw_ring_queue::clear() noexcept {
    head_ = 0;
    tail_ = 0;
    size_ = 0;
}
