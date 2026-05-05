#pragma once

#include <cstdint>

void riz_mock_tick_set(std::uint32_t t) noexcept;
void riz_mock_tick_advance(std::uint32_t dt) noexcept;
