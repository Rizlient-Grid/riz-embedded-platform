#pragma once

#include <riz/coro/execution/schedulable_node.h>
#include <riz/coro/execution/scheduler.h>
#include <riz/coro/promise/promise_base.hpp>

#include <coroutine>
#include <utility>

namespace riz::coro {
template<typename T>
class schedulable_task;
}

namespace riz::coro::promise {

template<typename T>
using schedulable_task = riz::coro::schedulable_task<T>;
template<typename T>
struct schedulable_task_promise;
using schedulable_node = execution::schedulable_node;
using scheduler = execution::scheduler;

template<typename T>
using schedulable_task_trait =
    resumable_trait<schedulable_task, schedulable_task_promise, T>;

template<typename T>
struct schedulable_task_promise
    : riz::coro::promise::promise_base<schedulable_task_trait<T>> {
    using resumable_type = schedulable_task<T>;

    scheduler& executor;
    schedulable_node schedulable_node;
    resumable_type::return_type result;

    template<typename... Ts>
    schedulable_task_promise(scheduler& sched, Ts&&...)
        : executor(sched)
        , schedulable_node {
              .coro_handle =
                  std::coroutine_handle<schedulable_task_promise>::from_promise(
                      *this)} {}

    void return_value(resumable_type::return_type&& value) {
        result = std::move(value);
    }

    void return_value(const resumable_type::return_type& value) {
        result = value;
    }
};

template<>
struct schedulable_task_promise<void>
    : riz::coro::promise::promise_base<schedulable_task_trait<void>> {
    using resumable_type = schedulable_task<void>;

    scheduler& executor;
    schedulable_node schedulable_node;

    template<typename... Ts>
    schedulable_task_promise(scheduler& sched, Ts&&...)
        : executor(sched)
        , schedulable_node {
              .coro_handle =
                  std::coroutine_handle<schedulable_task_promise>::from_promise(
                      *this)} {}

    void return_void() noexcept {}
};

} // namespace riz::coro::promise