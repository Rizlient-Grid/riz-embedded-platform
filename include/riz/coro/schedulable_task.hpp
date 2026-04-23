#pragma once

#include <riz/coro/promise.hpp>
#include <riz/coro/resumable.hpp>
#include <riz/coro/scheduler.hpp>

#include <coroutine>

namespace riz::coro {

template<typename T>
class schedulable_task;
template<typename T>
struct schedulable_task_promise;

template<typename T>
using schedulable_task_trait =
    resumable_trait<schedulable_task, schedulable_task_promise, T>;

template<typename T>
class schedulable_task : private resumable<schedulable_task_trait<T>> {
public:
    using return_type = T;
    using promise_type = schedulable_task_promise<return_type>;
    using scheduler_type = scheduler<>;
    using resumable<schedulable_task_trait<T>>::tag_is_resumable;

    explicit schedulable_task(std::coroutine_handle<promise_type> handle)
        : resumable<schedulable_task_trait<T>> {handle}
    {
    }

    using resumable<schedulable_task_trait<T>>::done;
    using resumable<schedulable_task_trait<T>>::take_result;

private:
    template<Resumable ResumableT>
    friend struct riz::coro::awaiter::resumable_awaiter;

    template<typename R>
    friend schedulable_task<R> start(schedulable_task<R>&& task);
};

template<typename T>
struct schedulable_task_promise : promise<schedulable_task_trait<T>> {
    using resumable_type = schedulable_task<T>;
    using scheduler_type = scheduler<>;

    scheduler_type& scheduler;
    scheduler_type::node_type schedulable_node;
    resumable_type::return_type result;

    template<typename... Ts>
    schedulable_task_promise(scheduler_type& sched, Ts&&...)
        : scheduler(sched)
        , schedulable_node {
              .coro_handle =
                  std::coroutine_handle<schedulable_task_promise>::from_promise(
                      *this)}
    {
    }

    void return_value(resumable_type::return_type&& value)
    {
        result = std::move(value);
    }

    void return_value(const resumable_type::return_type& value)
    {
        result = value;
    }
};

template<>
struct schedulable_task_promise<void> : promise<schedulable_task_trait<void>> {
    using resumable_type = schedulable_task<void>;
    using scheduler_type = scheduler<>;

    scheduler_type& scheduler;
    scheduler_type::node_type schedulable_node;

    template<typename... Ts>
    schedulable_task_promise(scheduler_type& sched, Ts&&...)
        : scheduler(sched)
        , schedulable_node {
              .coro_handle =
                  std::coroutine_handle<schedulable_task_promise>::from_promise(
                      *this)}
    {
    }

    void return_void() noexcept {}
};

} // namespace riz::coro
