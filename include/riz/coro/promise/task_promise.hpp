#pragma once

#include <riz/coro/constraint/resumable.hpp>
#include <riz/coro/promise/promise_base.hpp>

namespace riz::coro::resumable {
template<typename T>
class task;
template<typename T>
class schedulable_task;
} // namespace riz::coro::resumable

namespace riz::coro::promise {

template<typename T>
struct task_promise;
template<typename T>
using task_pair = resumable_pair<resumable::task, task_promise, T>;

template<typename T>
struct task_promise : promise_base<task_pair<T>> {
    using return_type = task_pair<T>::return_type;
    using resumable_type = task_pair<T>::resumable_type;

    return_type result;

    void return_value(return_type&& value) {
        result = std::move(value);
    }

    void return_value(const return_type& value) {
        result = value;
    }

    using promise_base<task_pair<T>>::await_transform;

    template<typename U>
    auto await_transform(resumable::schedulable_task<U>&&) = delete;
};

template<>
struct task_promise<void> : promise_base<task_pair<void>> {
    using resumable_type = task_pair<void>::resumable_type;

    void return_void() noexcept {}

    using promise_base<task_pair<void>>::await_transform;

    template<typename U>
    auto await_transform(resumable::schedulable_task<U>&&) = delete;
};

}; // namespace riz::coro::promise
