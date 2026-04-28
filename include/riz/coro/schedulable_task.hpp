#pragma once

#include <riz/coro/execution/scheduler.h>
#include <riz/coro/promise/schedulable_task_promise.hpp>
#include <riz/coro/resumable.hpp>

#include <coroutine>

namespace riz::coro::execution {
template<typename T>
riz::coro::schedulable_task<T> start(riz::coro::schedulable_task<T>&& task);
}

namespace riz::coro {

template<typename T>
using schedulable_task_trait = promise::schedulable_task_trait<T>;
template<typename T>
using schedulable_task_promise = promise::schedulable_task_promise<T>;

template<typename T>
class schedulable_task : private resumable<schedulable_task_trait<T>> {
public:
    using return_type = T;
    using promise_type = schedulable_task_promise<return_type>;
    using resumable<schedulable_task_trait<T>>::tag_is_resumable;

    explicit schedulable_task(std::coroutine_handle<promise_type> handle)
        : resumable<schedulable_task_trait<T>> {handle} {}

    using resumable<schedulable_task_trait<T>>::done;
    using resumable<schedulable_task_trait<T>>::take_result;

private:
    template<Resumable ResumableT>
    friend struct riz::coro::awaiter::resumable_awaiter;

    template<typename R>
    friend schedulable_task<R>
    riz::coro::execution::start(schedulable_task<R>&& task);
};

} // namespace riz::coro
