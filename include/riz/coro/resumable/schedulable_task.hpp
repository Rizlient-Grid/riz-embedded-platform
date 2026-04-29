#pragma once

#include <riz/coro/execution/scheduler.h>
#include <riz/coro/promise/schedulable_task_promise.hpp>
#include <riz/coro/resumable/resumable_base.hpp>

#include <coroutine>

namespace riz::coro::resumable {
template<typename T>
class schedulable_task;
} // namespace riz::coro::resumable

namespace riz::coro::execution {
template<typename T>
resumable::schedulable_task<T> start(resumable::schedulable_task<T>&& task);
} // namespace riz::coro::execution

namespace riz::coro::resumable {

template<typename T>
class schedulable_task
    : private resumable_base<promise::schedulable_task_pair<T>> {
public:
    using return_type = T;
    using promise_type = promise::schedulable_task_pair<T>::promise_type;
    using resumable_base<promise::schedulable_task_pair<T>>::tag_is_resumable;

    explicit schedulable_task(std::coroutine_handle<promise_type> handle)
        : resumable_base<promise::schedulable_task_pair<T>> {handle} {}

    using resumable_base<promise::schedulable_task_pair<T>>::done;
    using resumable_base<promise::schedulable_task_pair<T>>::take_result;

private:
    template<constraint::resumable ResumableT>
    friend struct awaiter::resumable_awaiter;

    template<typename R>
    friend schedulable_task<R> execution::start(schedulable_task<R>&& task);
};

} // namespace riz::coro::resumable
