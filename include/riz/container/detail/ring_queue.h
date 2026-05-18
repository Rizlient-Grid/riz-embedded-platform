#pragma once

#include <riz/constraints.h>

#include <cstddef>

namespace riz::container::detail {

class ring_queue : public noncopyable {
public:
    ring_queue(void* buffer, std::size_t entry_size, std::size_t capacity);

    ring_queue(ring_queue&& rhs) noexcept;

    ring_queue& operator=(ring_queue&& rhs) noexcept;

    [[nodiscard]] bool empty() const noexcept {
        return size_ == 0;
    }

    [[nodiscard]] bool full() const noexcept {
        return size_ == capacity_;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return size_;
    }

    [[nodiscard]] std::size_t capacity() const noexcept {
        return capacity_;
    }

    bool pop_front(void* value) noexcept;
    void push(const void* value) noexcept;
    void clear() noexcept;

private:
    std::byte* buffer_ {nullptr};
    std::size_t entry_size_ {0};
    std::size_t capacity_ {0};
    std::size_t head_ {0};
    std::size_t tail_ {0};
    std::size_t size_ {0};
};

} // namespace riz::container::detail
