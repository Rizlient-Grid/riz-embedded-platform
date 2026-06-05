#pragma once

#include "basic_channel_write_awaiter.h"

#include <cstddef>

namespace riz::coro::channel {
template<typename T>
    requires (std::is_trivially_copyable_v<T>)
class channel;
} // namespace riz::coro::channel

namespace riz::coro::awaiter {

template<typename T>
class channel_write_awaiter : public basic_channel_write_awaiter {
public:
    channel_write_awaiter(channel::channel<T>& chan, const T& value)
        : basic_channel_write_awaiter {chan, &value} {}
};

} // namespace riz::coro::awaiter
