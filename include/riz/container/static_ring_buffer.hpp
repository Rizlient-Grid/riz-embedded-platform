#pragma once

#include <riz/container/ring_queue.hpp>

#include <cstddef>

namespace riz::container {

template<typename T, std::size_t Capacity>
class static_ring_buffer {
public:
    static_ring_buffer()
        : buffer_ {storage_} {}

    [[nodiscard]] bool empty() const noexcept {
        return buffer_.empty();
    }

    [[nodiscard]] bool full() const noexcept {
        return buffer_.full();
    }

    void push(const T& value) noexcept {
        buffer_.push(value);
    }

    [[nodiscard]] bool pop_front(T& value) noexcept {
        return buffer_.pop_front(value);
    }

    void clear() noexcept {
        return buffer_.clear();
    }

private:
    ring_queue<T> buffer_;
    T storage_[Capacity];
};

template<typename T>
class static_ring_buffer<T, 0>{};

} // namespace riz::container
