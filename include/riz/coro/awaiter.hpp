#pragma once

#include <coroutine>

namespace riz::coro {

template<typename Pull, typename Push> struct promise;
template<typename Pull, typename Push> struct task;

struct final_awaiter
{
    final_awaiter() = delete;
    
    explicit final_awaiter(std::coroutine_handle<> continuation)
        : continuation_ { continuation }
    {
    }

    bool await_ready() const noexcept
    {
        return false;
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> h) noexcept
    {
        if (continuation_) {
            return continuation_;
        }
        return std::noop_coroutine();
    }

    void await_resume() noexcept
    {
    }

    std::coroutine_handle<> continuation_;
};

template<typename Task>
struct task_awaiter
{
    using task_type = Task;

    task_type& task_;
    
    explicit task_awaiter(task_type& task)
        : task_(task)
    {
    }
    
    bool await_ready() const noexcept
    {
        return false;
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> h) noexcept
    {
        task_.get_handle().promise().continuation_ = h;
        return task_.get_handle();
    }

    task_type::pull_type await_resume() noexcept
    {
        return std::move(task_.get_handle().promise().value_);
    }
};

} // namespace riz::coro