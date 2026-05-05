#pragma once

#include <cstdint>

namespace riz::hal {

class tick_source {
public:
    [[nodiscard]] std::uint32_t now() const noexcept;
    [[nodiscard]] std::uint32_t freq() const noexcept;
};

} // namespace riz::hal
