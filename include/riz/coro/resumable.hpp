#pragma once

#include <riz/constraints.h>

#include <cassert>
#include <coroutine>

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

template<typename ResumableT, typename PromiseT>
struct resumable_trait {
    using resumable_type = ResumableT;
    using promise_type = PromiseT;
};

template<ResumableTrait TraitT>
class resumable : public moveonly {
public:
    using resumable_trait_type = TraitT;
    using promise_type = resumable_trait_type::promise_type;
    static constexpr bool tag_is_resumable = true;

    explicit resumable(std::coroutine_handle<promise_type> h)
        : handle_ {h}
    {
    }

    ~resumable()
    {
        handle_.destroy();
    }

    resumable(resumable&&) = default;

    resumable& operator=(resumable&&) = default;

    void resume()
    {
        assert(!handle_.done());
        handle_.resume();
    }

    std::coroutine_handle<promise_type> handle() noexcept
    {
        return handle_;
    }

private:
    std::coroutine_handle<promise_type> handle_;
};

} // namespace riz::coro