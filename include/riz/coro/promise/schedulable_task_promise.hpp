#pragma once

#include <riz/coro/constraint/resumable.hpp>
#include <riz/coro/execution/schedulable_node.h>
#include <riz/coro/execution/scheduler.h>
#include <riz/coro/promise/promise_base.hpp>

#include <coroutine>
#include <utility>

namespace riz::coro::resumable {
template<typename T>
class schedulable_task;
} // namespace riz::coro::resumable

namespace riz::coro::promise {

template<typename T>
struct schedulable_task_promise;
template<typename T>
using schedulable_task_pair =
    promise::resumable_pair<resumable::schedulable_task,
                            schedulable_task_promise, T>;

template<typename T>
struct schedulable_task_promise : promise_base<schedulable_task_pair<T>> {
    using return_type = schedulable_task_pair<T>::return_type;
    using resumable_type = schedulable_task_pair<T>::resumable_type;

    execution::scheduler& executor;
    execution::schedulable_node schedulable_node;
    return_type result;

    template<typename... Ts>
    schedulable_task_promise(execution::scheduler& sched, Ts&&...)
        : executor(sched)
        , schedulable_node {
              .coro_handle =
                  std::coroutine_handle<schedulable_task_promise>::from_promise(
                      *this)} {}

    void return_value(return_type&& value) {
        result = std::move(value);
    }

    void return_value(const return_type& value) {
        result = value;
    }
};

template<>
struct schedulable_task_promise<void>
    : promise_base<schedulable_task_pair<void>> {
    using resumable_type = schedulable_task_pair<void>::resumable_type;

    execution::scheduler& executor;
    execution::schedulable_node schedulable_node;

    template<typename... Ts>
    schedulable_task_promise(execution::scheduler& sched, Ts&&...)
        : executor(sched)
        , schedulable_node {
              .coro_handle =
                  std::coroutine_handle<schedulable_task_promise>::from_promise(
                      *this)} {}

    void return_void() noexcept {}
};

} // namespace riz::coro::promise