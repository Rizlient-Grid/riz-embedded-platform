#pragma once

#include <coroutine>
#include <cstddef>

namespace riz::coro::channel {
template<typename T, std::size_t Capacity>
class channel;
} // namespace riz::coro::channel

namespace riz::coro::awaiter {

template<typename T, std::size_t Capacity>
class channel_send_awaiter {
public:
    channel_send_awaiter(const T& value, channel::channel<T, Capacity>& chan)
        : value_(value)
        , channel_(chan) {}

    bool await_ready() noexcept {
        if (channel_.closed()) {
            entry_.status = -1;
            return true;
        }
        if (channel_.try_send(value_)) {
            return true;
        }
        return false;
    }

    template<typename Promise>
    void await_suspend(std::coroutine_handle<Promise> handle) noexcept {
        entry_.value = const_cast<T*>(&value_);
        entry_.sched_node = &handle.promise().schedulable_node;
        channel_.push_pending_sender(entry_);
    }

    int await_resume() const noexcept {
        return entry_.status;
    }

private:
    using channel = channel::channel<T, Capacity>;

private:
    const T& value_;
    channel& channel_;
    channel::node entry_;
};

} // riz::coro::awaiter
