#pragma once

#include <riz/constraints.h>
#include <riz/container/intrusive/fifo_queue.h>
#include <riz/container/static_ring_buffer.hpp>
#include <riz/coro/awaiter/channel_receive_awaiter.hpp>
#include <riz/coro/awaiter/channel_send_awaiter.hpp>
#include <riz/coro/execution/schedulable_node.h>

#include <cstddef>
#include <cstring>
#include <type_traits>

namespace riz::coro::channel {

template<typename T, std::size_t Capacity = 0>
class channel : public immovable {
    static_assert(std::is_trivially_copyable_v<T>);

public:
    static constexpr std::size_t capacity() noexcept {
        return Capacity;
    }

    awaiter::channel_send_awaiter<T, Capacity> send(const T& value) noexcept {
        return {value, *this};
    }

    [[nodiscard]] bool try_send(const T& value) noexcept {
        if (closed_) {
            return false;
        }
        if (!pending_receivers_.empty()) {
            auto n = static_cast<node*>(pending_receivers_.pop_front());
            std::memcpy(n->value, &value, sizeof(T));
            n->sched_node->executor->post(*n->sched_node);
            return true;
        }
        if constexpr (Capacity > 0) {
            if (buffer_.full()) {
                return false;
            }
            buffer_.push(value);
            return true;
        }
        return false;
    }

    awaiter::channel_receive_awaiter<T, Capacity> receive(T& value) noexcept {
        return {value, *this};
    }
    
    [[nodiscard]] bool try_receive(T& value) noexcept {
        if constexpr (Capacity > 0) {
            if (!buffer_.empty()) {
                (void)buffer_.pop_front(value);
                if (auto n = static_cast<node*>(pending_senders_.pop_front())) {
                    buffer_.push(*n->value);
                    n->sched_node->executor->post(*n->sched_node);
                }
                return true;
            }
        }
        if (closed_) {
            return false;
        }
        if (!pending_senders_.empty()) {
            auto n = static_cast<node*>(pending_senders_.pop_front());
            std::memcpy(&value, n->value, sizeof(T));
            n->sched_node->executor->post(*n->sched_node);
            return true;
        }
        return false;
    }

    void close() noexcept {
        closed_ = true;
        while (auto n = static_cast<node*>(pending_senders_.pop_front())) {
            n->status = -1;
            n->sched_node->executor->post(*n->sched_node);
        }
        while (auto n = static_cast<node*>(pending_receivers_.pop_front())) {
            n->status = -1;
            n->sched_node->executor->post(*n->sched_node);
        }
    }

    [[nodiscard]] bool closed() const noexcept {
        return closed_;
    }

private:
    using fifo_queue = container::intrusive::fifo_queue;
    struct node : fifo_queue::node {
        execution::schedulable_node* sched_node {nullptr};
        T* value {nullptr};
        int status {0};
    };

private:
    void push_pending_sender(node& entry) noexcept {
        pending_senders_.push(entry);
    }

    void push_pending_receiver(node& entry) noexcept {
        pending_receivers_.push(entry);
    }

private:
    container::intrusive::fifo_queue pending_senders_;
    container::intrusive::fifo_queue pending_receivers_;
    [[no_unique_address]] container::static_ring_buffer<T, Capacity> buffer_;
    bool closed_ {false};

private:
    friend awaiter::channel_send_awaiter<T, Capacity>;
    friend awaiter::channel_receive_awaiter<T, Capacity>;
};

} // namespace riz::coro::channel
