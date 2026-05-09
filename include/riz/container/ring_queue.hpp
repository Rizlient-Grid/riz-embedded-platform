#pragma once

#include <riz/container/detail/ring_queue.h>

#include <cstddef>
#include <type_traits>

namespace riz::container {

template<typename T>
    requires std::is_trivially_copyable_v<T>
class ring_queue : public detail::ring_queue {
public:
    template<std::size_t Capacity>
        requires(Capacity > 0)
    ring_queue(T (&buffer)[Capacity])
        : detail::ring_queue {buffer, sizeof(T), Capacity} {}

    ring_queue(ring_queue&&) noexcept = default;

    ring_queue& operator=(ring_queue&&) noexcept = default;

    [[nodiscard]] bool empty() const noexcept {
        return detail::ring_queue::empty();
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return detail::ring_queue::size();
    }

    [[nodiscard]] std::size_t capacity() const noexcept {
        return detail::ring_queue::capacity();
    }

    bool pop_front(T& value) noexcept {
        return detail::ring_queue::pop_front(&value);
    }

    void push(const T& value) noexcept {
        detail::ring_queue::push(&value);
    }
};

} // namespace riz::container
