#pragma once

#include <riz/constraints.h>
#include <riz/container/lockfree/spsc_raw_byte_ring_buffer.h>
#include <riz/coro/awaiter/uart_receive_awaiter.h>
#include <riz/coro/awaiter/uart_transmit_awaiter.h>
#include <riz/coro/transport/read_acceptor.h>
#include <riz/coro/transport/write_acceptor.h>
#include <riz/errcode.h>
#include <riz/hal/uart.h>
#include <riz/timer/timer_node.h>

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace riz::io {

class uart_service : public immovable,
                     public coro::transport::read_acceptor,
                     public coro::transport::write_acceptor {
public:
	template<std::size_t N, std::size_t N2>
    uart_service(hal::uart& dev, std::byte (&storage)[N], std::byte (&storage2)[N2])
        : dev_ {dev}
	    , rx_ring_buffer_ {storage}
        , rx_buffer_ {storage2}
        , rx_buffer_size_ {N2} {}

    coro::awaiter::uart_receive_awaiter receive(
        void* data, std::size_t len, std::uint32_t timeout_ms = 0) noexcept;
    coro::awaiter::uart_transmit_awaiter transmit(
        const void* data, std::size_t len, std::uint32_t timeout_ms = 0) noexcept;

    void start() noexcept;
    void stop() noexcept;
    void on_tx_complete() noexcept;
    void on_rx_complete() noexcept;
    void on_rx_idle() noexcept;
    void on_rx_error() noexcept;

    void run() noexcept;

    bool try_fill_receiver(coro::awaiter::uart_receive_awaiter& receiver) noexcept;
    std::size_t try_drain_transmitter(coro::awaiter::uart_transmit_awaiter& transmitter,
        const std::byte*& data, std::size_t chunk_size) noexcept;

private:
    void complete_active_read(errcode status) noexcept;
    void complete_active_write(errcode status) noexcept;

    void process_write() noexcept;
    void process_read() noexcept;
    void process_error() noexcept;

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
    std::atomic<bool> write_ready_ {true};
    std::atomic<errcode> hw_status_ {errcode::success};
    coro::awaiter::uart_transmit_awaiter* active_write_awaiter_ {nullptr};
    coro::awaiter::uart_receive_awaiter* active_read_awaiter_ {nullptr};
    container::lockfree::spsc_raw_byte_ring_buffer rx_ring_buffer_;
    std::byte* rx_buffer_ {nullptr};
    const std::size_t rx_buffer_size_ {0};
    std::size_t rx_read_offset_ {0};
    tx_timeout_node tx_timer_;
    rx_timeout_node rx_timer_;
};

} // namespace riz::io
