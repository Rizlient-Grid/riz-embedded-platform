#pragma once

#include <riz/constraints.h>

#include <cstddef>
#include <type_traits>

namespace riz::container {

template<typename T>
    requires std::is_trivially_copyable_v<T>
class ring_queue : public immovable {
public:
    template<std::size_t N>
        requires(N > 0)
    ring_queue(T (&buffer)[N])
        : buffer_ {buffer}
        , capacity_ {N} {}

    [[nodiscard]] bool empty() const noexcept {
        return size_ == 0;
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return size_;
    }

    [[nodiscard]] std::size_t capacity() const noexcept {
        return capacity_;
    };

    bool pop_front(T& value) noexcept {
        if (empty()) {
            return false;
        }
        value = buffer_[head_];
        head_ = (head_ + 1) % capacity_;
        --size_;
        return true;
    }

    void push(const T& value) noexcept {
        const bool full = (size_ == capacity_);
        buffer_[tail_] = value;
        tail_ = (tail_ + 1) % capacity_;
        if (!full) {
            ++size_;
        } else {
            head_ = (head_ + 1) % capacity_;
        }
    }

private:
    T* buffer_ {nullptr};
    const std::size_t capacity_;
    std::size_t head_ {0};
    std::size_t tail_ {0};
    std::size_t size_ {0};
};

} // namespace riz::container
