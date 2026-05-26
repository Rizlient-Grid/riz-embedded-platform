#include <riz/hal/tick_source.h>

#include <stm32h7xx_hal.h>

using namespace riz::hal;

std::uint32_t tick_source::now() const noexcept {
    return HAL_GetTick();
}

std::uint32_t tick_source::freq() const noexcept {
    return 1000u;
}
