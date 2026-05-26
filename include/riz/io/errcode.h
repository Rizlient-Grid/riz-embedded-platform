#pragma once

#include <cstdint>

namespace riz::io {

enum class errcode : std::int8_t {
    success = 0,
    closed = -1,
    full = -2,
    empty = -3,
    canceled = -4,
    timeout = -5,
    hw_error = -6
};

} // namespace riz::io