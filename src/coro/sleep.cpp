#include <riz/coro/sleep.h>

riz::coro::awaiter::timer_awaiter riz::coro::sleep(std::uint32_t ticks) noexcept {
    return {ticks, true};
}
