#pragma once

#include <riz/coro/constraint/resumable.hpp>

#include <cassert>
#include <coroutine>
#include <type_traits>
#include <utility>

namespace riz::coro::awaiter {

template<constraint::resumable ResumableT>
struct resumable_awaiter {
    using resumable_type = ResumableT;

    resumable_type resumable_;

    template<typename T>
        requires constraint::resumable<std::remove_cvref_t<T>>
    explicit resumable_awaiter(T&& r)
        : resumable_(std::move(r)) {}

    bool await_ready() const noexcept {
        return false;
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> h) noexcept {
        assert(!resumable_.handle().promise().started);
        resumable_.handle().promise().continuation = h;
        return resumable_.handle();
    }

    resumable_type::return_type await_resume() noexcept {
        if constexpr (!std::is_void_v<typename resumable_type::return_type>) {
            return std::move(resumable_.handle().promise().result);
        }
    }
};

} // namespace riz::coro::awaiter