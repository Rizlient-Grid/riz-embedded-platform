#pragma once

#include <cstdint>

namespace riz::coro::channel {

enum class errcode : std::int8_t {
    success = 0,
    closed = -1,
    full = -2,
    empty = -3,
    canceled = -4
};

} // namespace riz::coro::channel
