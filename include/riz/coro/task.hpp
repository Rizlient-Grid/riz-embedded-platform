#pragma once

#include <riz/coro/promise.hpp>
#include <riz/coro/resumable.hpp>

#include <coroutine>

namespace riz::coro {

template<typename T>
class task;
template<typename T>
struct task_promise;
template<typename T>
class schedulable_task;

template<typename T>
using task_trait = resumable_trait<task, task_promise, T>;

template<typename T>
class task : public resumable<task_trait<T>> {
public:
    using return_type = T;
    using promise_type = task_promise<return_type>;

    explicit task(std::coroutine_handle<promise_type> handle)
        : resumable<task_trait<T>> {handle}
    {
    }
};

template<typename T>
struct task_promise : promise<task_trait<T>> {
    using resumable_type = task<T>;

    resumable_type::return_type result;

    void return_value(resumable_type::return_type&& value)
    {
        result = std::move(value);
    }

    void return_value(const resumable_type::return_type& value)
    {
        result = value;
    }

    using promise<task_trait<T>>::await_transform;

    template<typename U>
    auto await_transform(schedulable_task<U>&&) = delete;
};

template<>
struct task_promise<void> : promise<task_trait<void>> {
    using resumable_type = task<void>;

    void return_void() noexcept {}

    using promise<task_trait<void>>::await_transform;

    template<typename U>
    auto await_transform(schedulable_task<U>&&) = delete;
};

template<typename T>
[[nodiscard]] auto start(task<T>&& tsk)
{
    tsk.resume();
    return std::move(tsk);
}

} // namespace riz::coro
