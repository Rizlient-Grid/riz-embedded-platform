#pragma once

#include <riz/coro/channel/errcode.hpp>
#include <riz/coro/constraint/resumable.hpp>

#include <coroutine>
#include <cstddef>

namespace riz::coro::channel {
template<typename T, std::size_t Capacity>
class channel;
} // namespace riz::coro::channel

namespace riz::coro::awaiter {

template<typename T, std::size_t Capacity>
class channel_receive_awaiter {
public:
    channel_receive_awaiter(T& value, channel::channel<T, Capacity>& chan)
        : value_(value)
        , channel_(chan) {}

    bool await_ready() noexcept {
        channel::errcode ec = channel_.try_receive(value_);
        switch (ec) {
        case channel::errcode::success:
            return true;
        case channel::errcode::closed:
            entry_.status = ec;
            return true;
        default:
            return false;
        }
    }

    template<typename Promise>
        requires constraint::is_schedulable_task_promise_v<Promise>
    void await_suspend(std::coroutine_handle<Promise> handle) noexcept {
        entry_.value = &value_;
        entry_.sched_node = &handle.promise().schedulable_node;
        channel_.push_pending_receiver(entry_);
    }

    channel::errcode await_resume() const noexcept {
        return entry_.status;
    }

private:
    using channel_type = channel::channel<T, Capacity>;

private:
    T& value_;
    channel_type& channel_;
    channel_type::receiver_node entry_;
};

} // namespace riz::coro::awaiter
