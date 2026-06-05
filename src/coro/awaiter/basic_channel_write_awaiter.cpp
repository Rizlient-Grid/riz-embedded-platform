#include <riz/coro/awaiter/basic_channel_write_awaiter.h>
#include <riz/coro/channel/basic_channel.h>

using namespace riz::coro::awaiter;

bool basic_channel_write_awaiter::await_ready() noexcept {
    errcode ec = channel_.try_write(value_);
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

void basic_channel_write_awaiter::await_suspend(
    std::coroutine_handle<> handle) noexcept {
    auto base_handle = std::coroutine_handle<promise::schedulable_task_promise_base>::from_address(handle.address());
    sched_node_ = &base_handle.promise().schedulable_node;
    channel_.enqueue_pending_write(*this);
}

riz::errcode basic_channel_write_awaiter::await_resume() noexcept {
    return status_;
}
