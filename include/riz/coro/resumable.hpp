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

    std::coroutine_handle<promise_type> handle() noexcept
    {
        return handle_;
    }

private:
    std::coroutine_handle<promise_type> handle_;
};

} // namespace riz::coro
