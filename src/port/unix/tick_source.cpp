#include <riz/hal/tick_source.h>

#include <chrono>

using namespace riz::hal;

std::uint32_t tick_source::now() const noexcept {
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<std::uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

std::uint32_t tick_source::freq() const noexcept {
    return 1000u;
}
