#pragma once

#include "schedulable_awaiter_node.h"

#include <riz/errcode.h>

#include <coroutine>

namespace riz::coro::channel {
class basic_channel;
} // namespace riz::coro::channel

namespace riz::coro::awaiter {

class basic_channel_write_awaiter : public schedulable_awaiter_node {
public:
    basic_channel_write_awaiter(channel::basic_channel& chan, const void* value)
        : value_ {value}
        , channel_(chan) {}

    bool await_ready() noexcept;
    void await_suspend(std::coroutine_handle<> handle) noexcept;
    riz::errcode await_resume() noexcept;

private:
    const void* value_ {nullptr};
    channel::basic_channel& channel_;

private:
    friend channel::basic_channel;
};

} // namespace riz::coro::awaiter
