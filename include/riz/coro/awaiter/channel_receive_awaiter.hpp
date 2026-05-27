#pragma once

#include <riz/coro/channel/channel_node.h>
#include <riz/coro/channel/errcode.hpp>
#include <riz/coro/constraint/resumable.hpp>
#include <riz/coro/execution/schedulable_node.h>

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
            status_ = ec;
            return true;
        default:
            return false;
        }
    }

    template<typename Promise>
        requires constraint::is_schedulable_task_promise_v<Promise>
    void await_suspend(std::coroutine_handle<Promise> handle) noexcept {
        entry_.awaiter = this;
        entry_.sched_node = &handle.promise().schedulable_node;
        entry_.on_resume = &on_resume;
        channel_.push_pending_receiver(entry_);
    }

    channel::errcode await_resume() const noexcept {
        return status_;
    }

private:
    struct channel_receive_node : channel::channel_node {
        channel_receive_awaiter* awaiter {nullptr};
        execution::schedulable_node* sched_node {nullptr};
    };

    static void on_resume(channel::channel_node* node, void* data, channel::errcode status) noexcept {
        auto n = static_cast<channel_receive_node*>(node);
        auto awaiter = n->awaiter;
        awaiter->status_ = status;
        if (data != nullptr) {
            awaiter->value_ = *static_cast<T*>(data);
        }
        n->sched_node->executor->post(*n->sched_node);
    }

private:
    T& value_;
    channel::errcode status_ {channel::errcode::success};
    channel::channel<T, Capacity>& channel_;
    channel_receive_node entry_;
};

} // namespace riz::coro::awaiter
