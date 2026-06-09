#include <riz/coro/awaiter/basic_channel_read_awaiter.h>
#include <riz/coro/promise/schedulable_task_promise_base.h>
#include <riz/coro/channel/basic_channel.h>

using namespace riz::coro::awaiter;

bool basic_channel_read_awaiter::await_ready() noexcept {
    errcode ec = channel_.try_read(value_);
    switch (ec) {
    case errcode::success:
        return true;
    case errcode::closed:
        status_ = ec;
        return true;
    default:
        return false;
    }
    return false;
}

void basic_channel_read_awaiter::await_suspend(std::coroutine_handle<> handle) noexcept {
    auto base_handle = std::coroutine_handle<promise::schedulable_task_promise_base>::from_address(
        handle.address());
    sched_node_ = &base_handle.promise().schedulable_node;
    channel_.enqueue_pending_read(*this);
}

riz::errcode basic_channel_read_awaiter::await_resume() noexcept {
    return status_;
}
