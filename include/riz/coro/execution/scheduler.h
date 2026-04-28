#pragma once

#include <riz/constraints.h>
#include <riz/container/intrusive/fifo_queue.h>
#include <riz/coro/execution/schedulable_node.h>

#include <coroutine>
#include <type_traits>

namespace riz::coro::execution {

class scheduler : public noncopyable {
    using fifo_queue = riz::container::intrusive::fifo_queue;

public:
    using node_type = schedulable_node;
    scheduler() = default;
    void post(node_type& entry) noexcept;
    void run() noexcept;
    bool run_once() noexcept;

private:
    fifo_queue ready_queue_;
};

} // namespace riz::coro::execution
