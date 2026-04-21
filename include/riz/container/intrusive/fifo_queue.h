#pragma once

#include <riz/constraints.h>

#include <cstddef>

namespace riz::container::intrusive {

class fifo_queue : public noncopyable {
public:
    using node_type = struct node {
        node* next;
    };

    fifo_queue() = default;
    std::size_t size() const noexcept;
    bool empty() const noexcept;

    void push(node_type& entry) noexcept;
    [[nodiscard]] node_type* pop_front() noexcept;

private:
    node_type* head_ {nullptr};
    node_type* tail_ {nullptr};
    std::size_t size_ {0};
};

} // namespace riz::container::intrusive
