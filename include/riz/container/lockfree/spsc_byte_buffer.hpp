#pragma once

#include <riz/container/detail/lockfree/spsc_byte_ring_queue.h>

#include <cstddef>

namespace riz::container::lockfree {

template<std::size_t Capacity>
    requires (Capacity > 1)
class spsc_byte_buffer : public detail::lockfree::spsc_byte_ring_queue {
public:
    spsc_byte_buffer()
        : detail::lockfree::spsc_byte_ring_queue {storage_} {}
    
    constexpr std::size_t capacity() const noexcept {
        return Capacity;
    }

    std::size_t push(const std::byte* data, std::size_t len) noexcept {
        return detail::lockfree::spsc_byte_ring_queue::push(data, len);
    }

    std::size_t pop_front(std::byte* data, std::size_t len) noexcept {
        return detail::lockfree::spsc_byte_ring_queue::pop_front(data, len);
    }

    std::size_t size() const noexcept {
        return detail::lockfree::spsc_byte_ring_queue::size();
    }
private:
    std::byte storage_[Capacity];
};

} // namespace riz::container::lockfree
