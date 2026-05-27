#pragma once

#include <atomic>
#include <cstddef>

namespace riz::container::detail::lockfree {

class spsc_byte_ring_queue {
public:
    template<std::size_t Capacity>
        requires (Capacity > 1)
    spsc_byte_ring_queue(std::byte (&storage)[Capacity])
        : capacity_ {Capacity}
        , buff_ {storage} {}

    bool push(const std::byte* data, std::size_t len) noexcept;
    std::size_t pop_front(std::byte* data, std::size_t max_len) noexcept;
    std::size_t size() const noexcept;
    
    std::size_t capacity() const noexcept {
        return capacity_;
    }

private:
    const std::size_t capacity_;
    std::byte* buff_ {nullptr};
    std::atomic<std::size_t> head_ {0};
    std::atomic<std::size_t> tail_ {0};
};

} // namespace riz::container::detail::lockfree
