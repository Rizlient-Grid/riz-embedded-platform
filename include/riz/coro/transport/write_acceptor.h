#pragma once

#include <riz/container/intrusive/fifo_queue.h>
#include <riz/coro/awaiter/schedulable_awaiter_node.h>

namespace riz::coro::transport {

class write_acceptor {
public:
    using schedulable_awaiter_node = coro::awaiter::schedulable_awaiter_node;

public:
    void enqueue_pending_write(schedulable_awaiter_node& entry) noexcept {
        pending_writers_.push(entry);
    }

    schedulable_awaiter_node* dequeue_pending_write() noexcept {
        return static_cast<schedulable_awaiter_node*>(pending_writers_.pop_front());
    }

protected:
    container::intrusive::fifo_queue pending_writers_;
};

} // namespace riz::coro::transport
