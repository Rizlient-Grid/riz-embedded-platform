#pragma once

#include <riz/constraints.h>

#include <concepts>
#include <cstddef>
#include <cstdint>

namespace riz::container::intrusive {

class delta_queue : public noncopyable {
public:
    using delta_type = std::uint32_t;
    using key_type = delta_type;
    using node_type = struct node {
        delta_type delta;
        node* next;
    };

    delta_queue() = default;
    bool empty() const noexcept;
    std::size_t size() const noexcept;

    void insert(key_type abs_key, node_type& entry);
    bool erase(node_type& node) noexcept;
    node_type* pop_front() noexcept;

    template<typename Callback>
        requires std::invocable<Callback, node_type*>
    void advance(delta_type elapsed, Callback&& callback)
    {
        while (head_ && head_->delta <= elapsed) {
            elapsed -= head_->delta;
            node_type* entry = pop_front();
            callback(entry);
        }
        if (head_ && elapsed > 0) {
            head_->delta -= elapsed;
        }
    }

private:
    node_type* head_ {nullptr};
    std::size_t size_ {0u};
};

} // namespace riz::container