#pragma once

#include "basic_channel.h"

#include <riz/coro/awaiter/channel_read_awaiter.hpp>
#include <riz/coro/awaiter/channel_write_awaiter.hpp>
#include <riz/errcode.h>

#include <cstddef>
#include <type_traits>

namespace riz::coro::channel {

template<typename T>
    requires(std::is_trivially_copyable_v<T>)
class channel : public basic_channel {
public:
    channel()
        : basic_channel {sizeof(T)} {}

    template<std::size_t Capacity>
    channel(T (&storage)[Capacity])
        : basic_channel {storage} {}

    awaiter::channel_write_awaiter<T> send(const T& value) noexcept {
        return {*this, value};
    }

    awaiter::channel_read_awaiter<T> receive(T& value) noexcept {
        return {*this, value};
    }

    [[nodiscard]] errcode try_send(const T& value) noexcept {
        return basic_channel::try_write(&value);
    }

    [[nodiscard]] errcode try_read(T& value) noexcept {
        return basic_channel::try_read(&value);
    }
};

} // namespace riz::coro::channel
