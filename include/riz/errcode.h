#pragma once

#include <cstdint>

namespace riz {

enum errcode : std::int8_t {
    success = 0,
    closed = -1,
    full = -2,
    empty = -3,
    canceled = -4
};

} // namespace riz
