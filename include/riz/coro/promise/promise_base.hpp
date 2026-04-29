#pragma once

#include <riz/coro/awaiter/final_awaiter.hpp>
#include <riz/coro/awaiter/initial_awaiter.hpp>
#include <riz/coro/awaiter/resumable_awaiter.hpp>
#include <riz/coro/constraint/resumable.hpp>

#include <coroutine>
#include <exception>
#include <type_traits>

namespace riz::coro::promise {

template<template<typename> typename ResumableT,
         template<typename> typename PromiseT, typename T>
struct resumable_pair {
    using return_type = T;
    using resumable_type = ResumableT<return_type>;
    using promise_type = PromiseT<return_type>;
};

template<constraint::resumable_pair ResumablePairT>
struct promise_base {
    using resumable_pair_type = ResumablePairT;
    using resumable_type = resumable_pair_type::resumable_type;
    using promise_type = resumable_pair_type::promise_type;

    std::coroutine_handle<> continuation;
    bool started {false};

    resumable_type get_return_object() {
        auto handle = std::coroutine_handle<promise_type>::from_promise(
            static_cast<promise_type&>(*this));
        return resumable_type {handle};
    }

    auto initial_suspend() noexcept {
        return awaiter::initial_awaiter<promise_base> {*this};
    }

    auto final_suspend() noexcept {
        return awaiter::final_awaiter {continuation};
    }

    void unhandled_exception() noexcept {
        std::terminate();
    }

    template<constraint::resumable T>
    auto await_transform(T&) = delete;

    template<constraint::resumable T>
    auto await_transform(T&& awaitable) {
        return awaiter::resumable_awaiter<T> {std::move(awaitable)};
    }
};

} // namespace riz::coro::promise
