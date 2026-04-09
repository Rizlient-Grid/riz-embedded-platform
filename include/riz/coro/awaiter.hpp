#pragma once

#include <coroutine>

namespace riz::coro {

template<typename Pull, typename Push> struct promise;

template<typename Pull, typename Push>
struct final_awaiter
{
    using promise_type = promise<Pull, Push>;

    final_awaiter() : continuation_{} {}
    
    final_awaiter(std::coroutine_handle<promise_type> continuation)
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

    std::coroutine_handle<promise_type> continuation_;
};

} // namespace riz::coro