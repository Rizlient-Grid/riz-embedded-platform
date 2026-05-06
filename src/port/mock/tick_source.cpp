#include <riz/hal/tick_source.h>

#include <atomic>

namespace {

std::uint32_t g_mock_now {0};

} // namespace

namespace riz::hal {

std::uint32_t tick_source::now() const noexcept {
    return g_mock_now;
}

std::uint32_t tick_source::freq() const noexcept {
    return 1000u;
}

} // namespace riz::hal

void riz_mock_tick_set(std::uint32_t t) noexcept {
    g_mock_now = t;
}

void riz_mock_tick_advance(std::uint32_t dt) noexcept {
    g_mock_now += dt;
}
