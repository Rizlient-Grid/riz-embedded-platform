#pragma once

#include <riz/coro/promise/task_promise.hpp>
#include <riz/coro/resumable.hpp>

#include <coroutine>

namespace riz::coro {

template<typename T>
using task_trait = riz::coro::promise::task_trait<T>;

template<typename T>
class task : public resumable<task_trait<T>> {
public:
    using return_type = T;
    using promise_type = promise::task_promise<return_type>;

    explicit task(std::coroutine_handle<promise_type> handle)
        : resumable<task_trait<T>> {handle} {}
};

template<typename T>
[[nodiscard]] auto start(task<T>&& tsk) {
    tsk.resume();
    return std::move(tsk);
}

} // namespace riz::coro
