#pragma once

#include <riz/coro/awaiter/timer_awaiter.hpp>

#include <chrono>
#include <cstdint>

namespace riz::coro {

awaiter::timer_awaiter sleep(std::uint32_t ticks) noexcept {
    return awaiter::timer_awaiter {ticks, true};
}

template<typename Rep, typename Period>
awaiter::timer_awaiter sleep(std::chrono::duration<Rep, Period> dur) noexcept {
    auto ms = static_cast<std::uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(dur).count());
    return awaiter::timer_awaiter {ms};
}

} // namespace riz::coro
