#pragma once

#include <riz/coro/resumable.hpp>

#include <coroutine>
#include <type_traits>

namespace riz::coro::awaiter {

template<typename Promise>
struct initial_awaiter {
    using promise_type = Promise;

    promise_type& promise_;

    explicit initial_awaiter(promise_type& r)
        : promise_(r)
    {
    }

    bool await_ready() const noexcept
    {
        return false;
    }

    void await_suspend(std::coroutine_handle<>) noexcept {}

    void await_resume() noexcept
    {
        promise_.started_ = true;
    }
};

} // namespace riz::coro::awaiter
