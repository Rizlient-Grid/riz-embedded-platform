#pragma once

#include <riz/coro/promise/promise.hpp>
#include <riz/coro/resumable.hpp>

namespace riz::coro {
template<typename T>
class task;
template<typename T>
class schedulable_task;
} // namespace riz::coro

namespace riz::coro::promise {

template<typename T>
using task = riz::coro::task<T>;
template<typename T>
using schedulable_task = riz::coro::schedulable_task<T>;
template<typename T>
struct task_promise;

template<typename T>
using task_trait = resumable_trait<task, task_promise, T>;

template<typename T>
struct task_promise : promise_base<task_trait<T>> {
    using resumable_type = task<T>;

    resumable_type::return_type result;

    void return_value(resumable_type::return_type&& value) {
        result = std::move(value);
    }

    void return_value(const resumable_type::return_type& value) {
        result = value;
    }

    using promise_base<task_trait<T>>::await_transform;

    template<typename U>
    auto await_transform(schedulable_task<U>&&) = delete;
};

template<>
struct task_promise<void> : promise_base<task_trait<void>> {
    using resumable_type = task<void>;

    void return_void() noexcept {}

    using promise_base<task_trait<void>>::await_transform;

    template<typename U>
    auto await_transform(schedulable_task<U>&&) = delete;
};

}; // namespace riz::coro::promise
