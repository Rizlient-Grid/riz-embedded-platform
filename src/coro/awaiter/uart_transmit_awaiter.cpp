#include <riz/coro/awaiter/uart_transmit_awaiter.h>
#include <riz/coro/promise/schedulable_task_promise.hpp>
#include <riz/io/uart_service.h>

using namespace riz::coro::awaiter;

bool uart_transmit_awaiter::await_ready() noexcept {
    return false;
}

void uart_transmit_awaiter::await_suspend(std::coroutine_handle<> handle) noexcept {
    auto base_handle = std::coroutine_handle<promise::schedulable_task_promise_base>::from_address(
        handle.address());
    sched_node_ = &base_handle.promise().schedulable_node;
    svc_.enqueue_pending_write(*this);
}

riz::errcode uart_transmit_awaiter::await_resume() noexcept {
    return status_;
}