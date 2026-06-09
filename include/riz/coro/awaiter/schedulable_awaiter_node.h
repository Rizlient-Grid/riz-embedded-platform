#pragma once

#include <riz/container/intrusive/fifo_queue.h>
#include <riz/coro/execution/schedulable_node.h>
#include <riz/coro/execution/scheduler.h>
#include <riz/errcode.h>

namespace riz::coro::awaiter {

class schedulable_awaiter_node : public container::intrusive::fifo_queue::node_type {
public:
    void on_resume(errcode status) noexcept {
        status_ = status;
        sched_node_->executor->post(*sched_node_);
    }

protected:
    execution::schedulable_node* sched_node_ {nullptr};
    errcode status_ {errcode::success};
};

} // namespace riz::coro::awaiter
