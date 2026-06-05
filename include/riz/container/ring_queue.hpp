#pragma once

#include <riz/container/raw_ring_queue.h>

#include <cstddef>
#include <type_traits>

namespace riz::container {

template<typename T>
    requires std::is_trivially_copyable_v<T>
class ring_queue : public raw_ring_queue {
public:
    template<std::size_t Capacity>
        requires(Capacity > 0)
    ring_queue(T (&buffer)[Capacity])
        : raw_ring_queue {buffer, sizeof(T), Capacity} {}

    ring_queue(ring_queue&&) noexcept = default;

    ring_queue& operator=(ring_queue&&) noexcept = default;

    [[nodiscard]] bool empty() const noexcept {
        return raw_ring_queue::empty();
    }

    [[nodiscard]] bool full() const noexcept {
        return raw_ring_queue::full();
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return raw_ring_queue::size();
    }

    [[nodiscard]] std::size_t capacity() const noexcept {
        return raw_ring_queue::capacity();
    }

    bool pop_front(T& value) noexcept {
        return raw_ring_queue::pop_front(&value);
    }

    void push(const T& value) noexcept {
        raw_ring_queue::push(&value);
    }

    void clear() noexcept {
        raw_ring_queue::clear();
    }
};

} // namespace riz::container
