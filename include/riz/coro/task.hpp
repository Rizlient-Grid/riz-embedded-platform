#pragma once

#include "riz/constraints.h"

#include <coroutine>

namespace riz::coro {

template<typename Pull, typename Push>
struct promise;

template<typename Pull, typename Push>
class task : noncopyable
{
public:
    using promise_type = promise<Pull, Push>;

    explicit task(std::coroutine_handle<promise_type> handle)
        : handle_(handle)
    {
    }

    ~task()
    {
        handle_.destroy();
    }

    void resume()
    {
        handle_.resume();
    }

    std::coroutine_handle<promise_type> get_handle()
    {
        return handle_;
    }

private:
    std::coroutine_handle<promise_type> handle_;
};

} // namespace riz::coro