#pragma once

#include <riz/container/ring_queue.hpp>

#include <cstddef>

namespace riz::container {

template<typename T, std::size_t Capacity>
class static_ring_buffer : public ring_queue<T> {
public:
    static_ring_buffer()
        : ring_queue<T> {storage_} {}

private:
    T storage_[Capacity];
};

template<typename T>
class static_ring_buffer<T, 0> {};

} // namespace riz::container
