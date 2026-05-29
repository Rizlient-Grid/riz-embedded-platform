#pragma once

#include <riz/container/intrusive/fifo_queue.h>
#include <riz/coro/constraint/resumable.hpp>
#include <riz/coro/execution/schedulable_node.h>
#include <riz/coro/execution/scheduler.h>
#include <riz/coro/promise/schedulable_task_promise.hpp>
#include <riz/io/errcode.h>
#include <riz/io/uart_service.h>

#include <algorithm>
#include <coroutine>
#include <cstddef>

namespace riz::coro::awaiter {

class uart_receive_awaiter
    : public container::intrusive::fifo_queue::node_type {
public:
    uart_receive_awaiter(io::uart_service& svc, void* data, std::size_t len, std::uint32_t timeout_ms = 0)
        : svc_(svc)
        , data_ {static_cast<std::byte*>(data)}
        , len_ {len}
        , timeout_ms_ {timeout_ms} {}

    bool await_ready() noexcept {
        return svc_.try_fill_receiver(*this);
    }

    template<typename Promise>
        requires constraint::is_schedulable_task_promise_v<Promise>
    void await_suspend(std::coroutine_handle<Promise> handle) noexcept {
        sched_node_ = &handle.promise().schedulable_node;
        svc_.push_pending_receiver(*this);
    }

    io::errcode await_resume() noexcept {
        return status_;
    }

private:
    void on_resume(io::errcode status) noexcept {
        status_ = status;
        sched_node_->executor->post(*sched_node_);
    }

private:
    io::uart_service& svc_;
    std::byte* data_ {nullptr};
    std::size_t len_ {0};
    std::size_t offset_ {0};
    io::errcode status_ {io::errcode::success};
    std::uint32_t timeout_ms_ {0};
    execution::schedulable_node* sched_node_ {nullptr};

private:
    friend class io::uart_service;
};

}
