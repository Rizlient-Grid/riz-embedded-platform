#pragma once

#include <cstddef>

namespace riz::container {

class byte_ring_buffer {
public:
    template<std::size_t Capacity>
        requires (Capacity > 1)
    byte_ring_buffer(std::byte (&storage)[Capacity])
        : capacity_ {Capacity}
        , buff_ {storage} {}

    std::size_t push(const std::byte* data, std::size_t len) noexcept;
    std::size_t pop_front(std::byte* data, std::size_t max_len) noexcept;
    std::size_t size() const noexcept;
    
    std::size_t capacity() const noexcept {
        return capacity_;
    }

private:
    const std::size_t capacity_;
    std::byte* buff_ {nullptr};
    std::size_t head_ {0};
    std::size_t tail_ {0};
};

} // namespace riz::container
