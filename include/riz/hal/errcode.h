#pragma once

#include <riz/concepts/bitmask_enum.h>

#include <cstdint>

namespace riz::hal {

enum errcode : std::uint8_t {
    ok       = 0x00,
    framing  = 0x01,
    parity   = 0x02,
    overflow = 0x04,
    noise    = 0x08,
    dma      = 0x10,
};

}

template<>
struct riz::enable_bitmask<riz::hal::errcode> : std::true_type {};
