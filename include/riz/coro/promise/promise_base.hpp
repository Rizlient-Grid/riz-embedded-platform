#pragma once

#include <riz/coro/awaiter/final_awaiter.hpp>
#include <riz/coro/awaiter/initial_awaiter.hpp>
#include <riz/coro/awaiter/resumable_awaiter.hpp>
#include <riz/coro/resumable.hpp>

#include <coroutine>
#include <exception>
#include <type_traits>

namespace riz::coro::promise {

template<ResumableTrait TraitT>
struct promise_base {
    using resumable_trait_type = TraitT;
    using resumable_type = resumable_trait_type::resumable_type;
    using promise_type = resumable_trait_type::promise_type;

    std::coroutine_handle<> continuation_;
    bool started_ {false};

    resumable_type get_return_object() {
        auto handle = std::coroutine_handle<promise_type>::from_promise(
            static_cast<promise_type&>(*this));
        return resumable_type {handle};
    }

    auto initial_suspend() noexcept {
        return awaiter::initial_awaiter<promise_base<resumable_trait_type>> {
            *this};
    }

    auto final_suspend() noexcept {
        return awaiter::final_awaiter {continuation_};
    }

    void unhandled_exception() noexcept {
        std::terminate();
    }

    template<Resumable T>
    auto await_transform(T&) = delete;

    template<Resumable T>
    auto await_transform(T&& awaitable) {
        return awaiter::resumable_awaiter<T> {std::move(awaitable)};
    }
};

} // namespace riz::coro::promise
