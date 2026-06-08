#pragma once

#include <riz/errcode.h>

#include <cstddef>

namespace riz::hal {

class uart_observer {
public:
    virtual void on_tx_complete() noexcept = 0;
    virtual void on_rx_complete(const std::byte* data, std::size_t len) noexcept = 0;
    virtual void on_rx_idle(const std::byte* data, std::size_t len) noexcept = 0;
    virtual void on_rx_error(errcode err) noexcept = 0;

protected:
    ~uart_observer() = default;
};

} // namespace riz::hal
