#pragma once

#include <riz/constraints.h>

#include <cassert>
#include <coroutine>
#include <utility>

namespace riz::coro {

template<typename T>
concept Resumable = requires {
    typename T::promise_type;
    requires T::tag_is_resumable;
};

template<typename T>
concept ResumableTrait = requires {
    typename T::resumable_type;
    typename T::promise_type;
};

template<template<typename> typename ResumableT,
         template<typename> typename PromiseT,
         typename T>
struct resumable_trait {
    using return_type = T;
    using resumable_type = ResumableT<return_type>;
    using promise_type = PromiseT<return_type>;
};

template<ResumableTrait TraitT>
class resumable : public moveonly {
public:
    using resumable_trait_type = TraitT;
    using return_type = resumable_trait_type::return_type;
    using resumable_type = resumable_trait_type::resumable_type;
    using promise_type = resumable_trait_type::promise_type;
    static constexpr bool tag_is_resumable = true;

    explicit resumable(std::coroutine_handle<promise_type> h)
        : handle_ {h}
    {
    }

    ~resumable()
    {
        if (handle_) {
            handle_.destroy();
        }
    }

    resumable(resumable&& r)
        : handle_ {std::exchange(r.handle_, {})}
    {
    }

    resumable& operator=(resumable&& r)
    {
        if (this == &r) {
            return *this;
        }

        if (handle_) {
            handle_.destroy();
        }
        
        handle_ = std::exchange(r.handle_, {});
        return *this;
    }

    void resume()
    {
        assert(!handle_.done());
        handle_.resume();
    }

    bool done() const noexcept
    {
        return handle_.done();
    }

    template<typename R = return_type>
        requires (!std::is_void_v<R>)
    R take_result()
    {
        return std::move(handle_.promise().result);
    }

    std::coroutine_handle<promise_type> handle() noexcept
    {
        return handle_;
    }

private:
    std::coroutine_handle<promise_type> handle_;
};

} // namespace riz::coro
