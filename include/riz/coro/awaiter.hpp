#pragma once

#include <riz/coro/resumable.hpp>

#include <coroutine>

namespace riz::coro {

template<ResumableTrait T>
struct promise;
template<typename T>
struct task;

struct final_awaiter
{
    explicit final_awaiter(std::coroutine_handle<> continuation)
        : continuation_ {continuation}
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

    void await_resume() noexcept {}

    std::coroutine_handle<> continuation_;
};

template<typename Resumable>
struct resumable_awaiter
{
    using resumable_type = Resumable;

    resumable_type& resumable_;

    template<typename T>
    explicit resumable_awaiter(T&& r)
        : resumable_(r)
    {
    }

    bool await_ready() const noexcept
    {
        return false;
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> h) noexcept
    {
        resumable_.handle().promise().continuation_ = h;
        return resumable_.handle();
    }

    resumable_type::return_type await_resume() noexcept
    {
        if constexpr (!std::is_void_v<typename resumable_type::return_type>) {
            return std::move(resumable_.handle().promise().value_);
        }
    }
};

} // namespace riz::coro