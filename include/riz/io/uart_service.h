#pragma once

#include <riz/constraints.h>
#include <riz/container/intrusive/fifo_queue.h>
#include <riz/container/lockfree/spsc_byte_buffer.hpp>
#include <riz/hal/uart.h>
#include <riz/io/errcode.h>
#include <riz/timer/timer_node.h>

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace riz::coro::awaiter {
class uart_transmit_awaiter;
class uart_receive_awaiter;
} // namespace riz::coro::awaiter

namespace riz::io {

class uart_service : immovable, public hal::uart_observer {
public:
    uart_service(hal::uart& dev)
        : dev_(dev) {
        dev_.set_observer(this);
    }

    coro::awaiter::uart_receive_awaiter receive(void* data, std::size_t len, std::uint32_t timeout_ms = 0) noexcept;
    coro::awaiter::uart_transmit_awaiter transmit(const void* data, std::size_t len, std::uint32_t timeout_ms = 0) noexcept;

    bool try_fill_receiver(coro::awaiter::uart_receive_awaiter& receiver) noexcept;
    void push_pending_sender(container::intrusive::fifo_queue::node& node) noexcept;
    void push_pending_receiver(container::intrusive::fifo_queue::node& node) noexcept;
    void run() noexcept;

private:
    void on_tx_complete() noexcept override;
    void on_rx_complete(const std::byte* data, std::size_t len) noexcept override;
    void on_rx_idle(const std::byte* data, std::size_t len) noexcept override;
    void on_rx_error(hal::errcode err) noexcept override;

    static void on_tx_timeout(timer::timer_node* tn) noexcept;
    static void on_rx_timeout(timer::timer_node* tn) noexcept;

private:
    struct tx_timeout_node : timer::timer_node {
        uart_service* self;
    };

    struct rx_timeout_node : timer::timer_node {
        uart_service* self;
    };

private:
    hal::uart& dev_;
    std::atomic<bool> tx_ready_ {true};
    std::atomic<hal::errcode> hw_status_ {hal::errcode::ok};
    container::intrusive::fifo_queue pending_senders_;
    container::intrusive::fifo_queue pending_receivers_;
    coro::awaiter::uart_transmit_awaiter* active_sender_awaiter_ {nullptr};
    coro::awaiter::uart_receive_awaiter* active_receiver_awaiter_ {nullptr};
    tx_timeout_node tx_timer_;
    rx_timeout_node rx_timer_;
    container::lockfree::spsc_byte_buffer<128> rx_ring_buffer_;
};

}
