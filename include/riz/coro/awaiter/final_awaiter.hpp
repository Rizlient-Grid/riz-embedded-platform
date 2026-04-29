#pragma once

#include <coroutine>
#include <type_traits>

namespace riz::coro::awaiter {

struct final_awaiter {
    explicit final_awaiter(std::coroutine_handle<> continuation)
        : continuation_ {continuation} {}

    bool await_ready() const noexcept {
        return false;
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> h) noexcept {
        if (continuation_) {
            return continuation_;
        }
        return std::noop_coroutine();
    }

    void await_resume() noexcept {}

    std::coroutine_handle<> continuation_;
};

} // namespace riz::coro::awaiter
