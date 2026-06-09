#pragma once

#include <riz/container/intrusive/fifo_queue.h>
#include <riz/coro/awaiter/schedulable_awaiter_node.h>

namespace riz::coro::transport {

class read_acceptor {
public:
    using schedulable_awaiter_node = coro::awaiter::schedulable_awaiter_node;

public:
    void enqueue_pending_read(schedulable_awaiter_node& entry) noexcept {
        pending_readers_.push(entry);
    }

    schedulable_awaiter_node* dequeue_pending_read() noexcept {
        return static_cast<schedulable_awaiter_node*>(pending_readers_.pop_front());
    }

protected:
    container::intrusive::fifo_queue pending_readers_;
};

} // namespace riz::coro::transport
