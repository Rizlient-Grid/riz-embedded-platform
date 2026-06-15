#pragma once

#include "schedulable_awaiter_node.h"

#include <riz/coro/promise/schedulable_task_promise.hpp>

#include <coroutine>

namespace riz::coro::awaiter {

class yield_awaiter : public schedulable_awaiter_node {
public:
    bool await_ready() const noexcept { return false; }

    void await_suspend(std::coroutine_handle<> handle) noexcept {
        auto base_handle = std::coroutine_handle<promise::schedulable_task_promise_base>::from_address(
            handle.address());
        sched_node_ = &base_handle.promise().schedulable_node;
        auto executor = sched_node_->executor;
        executor->post(*sched_node_);
    }

    void await_resume() noexcept {};
};

} // namespace riz::coro::awaiter
