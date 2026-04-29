#pragma once

#include <riz/coro/promise/task_promise.hpp>
#include <riz/coro/resumable/resumable_base.hpp>

#include <coroutine>

namespace riz::coro::resumable {

template<typename T>
class task : public resumable_base<promise::task_pair<T>> {
public:
    using return_type = T;
    using promise_type = promise::task_pair<T>::promise_type;

    explicit task(std::coroutine_handle<promise_type> handle)
        : resumable_base<promise::task_pair<T>> {handle} {}
};

template<typename T>
[[nodiscard]] auto start(task<T>&& tsk) {
    tsk.resume();
    return std::move(tsk);
}

} // namespace riz::coro::resumable
