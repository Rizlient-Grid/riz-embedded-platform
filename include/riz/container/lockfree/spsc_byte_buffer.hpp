#pragma once

#include <riz/container/lockfree/spsc_raw_byte_ring_buffer.h>

#include <cstddef>

namespace riz::container::lockfree {

template<std::size_t Capacity>
    requires(Capacity > 1)
class spsc_byte_buffer : public spsc_raw_byte_ring_buffer {
public:
    spsc_byte_buffer()
        : spsc_raw_byte_ring_buffer {storage_} {}

    constexpr std::size_t capacity() const noexcept {
        return Capacity;
    }

    std::size_t push(const std::byte* data, std::size_t len) noexcept {
        return spsc_raw_byte_ring_buffer::push(data, len);
    }

    std::size_t pop_front(std::byte* data, std::size_t len) noexcept {
        return spsc_raw_byte_ring_buffer::pop_front(data, len);
    }

    std::size_t size() const noexcept {
        return spsc_raw_byte_ring_buffer::size();
    }

private:
    std::byte storage_[Capacity];
};

} // namespace riz::container::lockfree
