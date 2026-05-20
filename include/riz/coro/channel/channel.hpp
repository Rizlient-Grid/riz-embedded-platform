#pragma once

#include <riz/constraints.h>
#include <riz/container/intrusive/fifo_queue.h>
#include <riz/container/static_ring_buffer.hpp>
#include <riz/coro/awaiter/channel_receive_awaiter.hpp>
#include <riz/coro/awaiter/channel_send_awaiter.hpp>
#include <riz/coro/channel/errcode.hpp>
#include <riz/coro/execution/schedulable_node.h>

#include <cstddef>
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

    [[nodiscard]] errcode try_send(const T& value) noexcept {
        if (closed_) {
            return errcode::closed;
        }
        if (auto n = static_cast<receiver_node*>(pending_receivers_.pop_front())) {
            *n->value = value;
            n->sched_node->executor->post(*n->sched_node);
            return errcode::success;
        }
        if constexpr (Capacity > 0) {
            if (buffer_.full()) {
                return errcode::full;
            }
            buffer_.push(value);
            return errcode::success;
        }
        return errcode::full;
    }

    awaiter::channel_receive_awaiter<T, Capacity> receive(T& value) noexcept {
        return {value, *this};
    }

    [[nodiscard]] errcode try_receive(T& value) noexcept {
        if constexpr (Capacity > 0) {
            if (buffer_.pop_front(value)) {
                if (auto n = static_cast<sender_node*>(pending_senders_.pop_front())) {
                    buffer_.push(*n->value);
                    n->sched_node->executor->post(*n->sched_node);
                }
                return errcode::success;
            }
        }
        if (closed_) {
            return errcode::closed;
        }
        if (auto n = static_cast<sender_node*>(pending_senders_.pop_front())) {
            value = *n->value;
            n->sched_node->executor->post(*n->sched_node);
            return errcode::success;
        }
        return errcode::empty;
    }

    void close() noexcept {
        closed_ = true;
        while (auto n = static_cast<sender_node*>(pending_senders_.pop_front())) {
            n->status = errcode::canceled;
            n->sched_node->executor->post(*n->sched_node);
        }
        while (auto n = static_cast<receiver_node*>(pending_receivers_.pop_front())) {
            n->status = errcode::canceled;
            n->sched_node->executor->post(*n->sched_node);
        }
    }

    [[nodiscard]] bool closed() const noexcept {
        return closed_;
    }

private:
    using fifo_queue = container::intrusive::fifo_queue;
    struct sender_node : fifo_queue::node {
        execution::schedulable_node* sched_node {nullptr};
        const T* value {nullptr};
        errcode status {errcode::success};
    };
    struct receiver_node : fifo_queue::node {
        execution::schedulable_node* sched_node {nullptr};
        T* value {nullptr};
        errcode status {errcode::success};
    };

private:
    void push_pending_sender(sender_node& entry) noexcept {
        pending_senders_.push(entry);
    }

    void push_pending_receiver(receiver_node& entry) noexcept {
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
