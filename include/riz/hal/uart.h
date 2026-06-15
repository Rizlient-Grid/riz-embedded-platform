#pragma once

#include <riz/constraints.h>
#include <riz/hal/uart_observer.h>

#include <cstddef>
#include <cstdint>

namespace riz::hal {

class uart : public immovable {
public:
    enum class transfer_mode : std::uint8_t
    {
        dma,
        it
    };

public:
    template<typename T>
    uart(T& dev, transfer_mode mode)
        : dev_ {&dev}
        , mode_ {mode} {}

    void enable_irq() noexcept;
    void disable_irq() noexcept;
    int start_receive(void* data, std::size_t len) noexcept;
    int start_transmit(const void* data, std::size_t len) noexcept;
    int abort_transmit() noexcept;
    std::size_t get_rx_transfer_remaining() noexcept;
    errcode get_and_clear_hw_error() noexcept;

private:
    void* dev_;
    const transfer_mode mode_;
};

} // namespace riz::hal
