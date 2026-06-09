#pragma once

#include "basic_channel_read_awaiter.h"

#include <cstddef>

namespace riz::coro::channel {
template<typename T>
    requires(std::is_trivially_copyable_v<T>)
class channel;
} // namespace riz::coro::channel

namespace riz::coro::awaiter {

template<typename T>
class channel_read_awaiter : public basic_channel_read_awaiter {
public:
    channel_read_awaiter(channel::channel<T>& chan, T& value)
        : basic_channel_read_awaiter {chan, &value} {}
};

} // namespace riz::coro::awaiter
