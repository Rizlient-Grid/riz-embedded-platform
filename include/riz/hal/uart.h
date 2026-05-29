#pragma once

#include <riz/constraints.h>
#include <riz/hal/uart_observer.h>

#include <cstddef>
#include <cstdint>

namespace riz::hal {

class uart : public immovable {
public:
    enum class transfer_mode : std::uint8_t { dma, it };

public:
    template<typename T, std::size_t N>
    uart(T& dev, transfer_mode mode, std::byte (&rx_buffer)[N])
        : dev_ {&dev}
        , mode_ {mode}
        , rx_buffer_ {rx_buffer}
        , rx_buffer_size_ {N} {}

    void enable_irq() noexcept;
    void disable_irq() noexcept;
    int start_receive(void* data, std::size_t len) noexcept;
    int start_transmit(const void* data, std::size_t len) noexcept;
    int abort_transmit() noexcept;
    void set_observer(uart_observer* obs) noexcept;

    void on_transmit_complete_isr() noexcept;
    void on_receive_complete_isr() noexcept;
    void on_receive_idle_isr() noexcept;
    void on_error_isr() noexcept;

private:
    void* dev_;
    const transfer_mode mode_;
    std::byte* rx_buffer_ {nullptr};
    const std::size_t rx_buffer_size_;
    std::size_t rx_read_offset_ {0};
    uart_observer* observer_ {nullptr};
};

}
