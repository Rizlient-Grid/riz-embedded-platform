#pragma once

#include "awaiter.hpp"
#include "task.hpp"

#include <coroutine>
#include <type_traits>

namespace riz::coro {

template<typename Pull, typename Push, typename PromiseDerived>
struct promise_base
{
    using task_type = task<Pull, Push>;
    using push_type = Push;
    using pull_type = Pull;
    using final_awaiter_type = final_awaiter<pull_type, push_type>;
    using promise_derived = PromiseDerived;
    
    task_type get_return_object()
    {
        auto handle = std::coroutine_handle<promise_derived>::from_promise(
            static_cast<promise_derived&>(*this));
        return task_type { handle };
    }

    std::suspend_always initial_suspend() noexcept
    {
        return {};
    }

    final_awaiter_type final_suspend() noexcept
    {
        return {};
    }

    void unhandled_exception()
    {
    }
};

template<typename Pull, typename Push>
struct promise : promise_base<Pull, Push, promise<Pull, Push>>
{
    using pull_type = Pull;
    using push_type = Push;

    pull_type value_;

    std::suspend_always yield_value(pull_type&& value) noexcept
    {
        value_ = std::move(value);
        return {};
    }

    void return_value(pull_type&& value)
    {
        value_ = std::move(value);
    }
};

template<typename Push>
struct promise<void, Push> : promise_base<void, Push, promise<void, Push>>
{
    void return_void() noexcept
    {
    }
};

} // namespace riz::coro