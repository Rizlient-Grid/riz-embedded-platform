#pragma once

#include <riz/coro/awaiter.hpp>
#include <riz/coro/resumable.hpp>

#include <coroutine>
#include <exception>
#include <type_traits>

namespace riz::coro {

template<ResumableTrait TraitT>
struct promise {
    using resumable_trait_type = TraitT;
    using resumable_type = resumable_trait_type::resumable_type;
    using promise_type = resumable_trait_type::promise_type;

    std::coroutine_handle<> continuation_;

    resumable_type get_return_object()
    {
        auto handle =
            std::coroutine_handle<promise_type>::from_promise(static_cast<promise_type&>(*this));
        return resumable_type {handle};
    }

    std::suspend_always initial_suspend() noexcept
    {
        return {};
    }

    auto final_suspend() noexcept
    {
        return final_awaiter {continuation_};
    }

    void unhandled_exception() noexcept
    {
        std::terminate();
    }

    template<Resumable T>
    auto await_transform(T&& awaitable)
    {
        return resumable_awaiter<T> {awaitable};
    }
};

} // namespace riz::coro
