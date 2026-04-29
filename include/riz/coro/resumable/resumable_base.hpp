#pragma once

#include <riz/constraints.h>
#include <riz/coro/constraint/resumable.hpp>

#include <cassert>
#include <coroutine>
#include <utility>

namespace riz::coro::resumable {

template<constraint::resumable_pair ResumablePairT>
class resumable_base : public moveonly {
public:
    using resumable_pair_type = ResumablePairT;
    using return_type = resumable_pair_type::return_type;
    using resumable_type = resumable_pair_type::resumable_type;
    using promise_type = resumable_pair_type::promise_type;
    static constexpr bool tag_is_resumable = true;

    explicit resumable_base(std::coroutine_handle<promise_type> h)
        : handle_ {h} {}

    ~resumable_base() {
        if (handle_) {
            handle_.destroy();
        }
    }

    resumable_base(resumable_base&& r)
        : handle_ {std::exchange(r.handle_, {})} {}

    resumable_base& operator=(resumable_base&& r) noexcept {
        if (this == &r) {
            return *this;
        }

        if (handle_) {
            handle_.destroy();
        }

        handle_ = std::exchange(r.handle_, {});
        return *this;
    }

    void resume() noexcept {
        assert(!handle_.done());
        handle_.resume();
    }

    bool done() const noexcept {
        return handle_.done();
    }

    template<typename R = return_type>
        requires(!std::is_void_v<R>)
    R take_result() noexcept {
        return std::move(handle_.promise().result);
    }

    std::coroutine_handle<promise_type> handle() noexcept {
        return handle_;
    }

    promise_type& promise() noexcept {
        return handle_.promise();
    }

private:
    std::coroutine_handle<promise_type> handle_;
};

} // namespace riz::coro::resumable
