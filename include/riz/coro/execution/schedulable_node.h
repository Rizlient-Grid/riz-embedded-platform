#pragma once

#include <riz/container/intrusive/fifo_queue.h>

#include <coroutine>

namespace riz::coro::execution {

class scheduler;
using fifo_queue_type = riz::container::intrusive::fifo_queue;

struct schedulable_node : fifo_queue_type::node_type {
    scheduler* executor;
    std::coroutine_handle<> coro_handle;
};

} // namespace riz::coro::execution
