#pragma once

#include <riz/container/intrusive/fifo_queue.h>

#include <coroutine>

namespace riz::coro::execution {

using fifo_queue_type = riz::container::intrusive::fifo_queue;

struct schedulable_node : fifo_queue_type::node_type {
    std::coroutine_handle<> coro_handle;
};

} // namespace riz::coro::execution
