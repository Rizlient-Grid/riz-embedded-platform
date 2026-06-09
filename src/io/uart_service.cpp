#include <riz/coro/awaiter/uart_receive_awaiter.h>
#include <riz/coro/awaiter/uart_transmit_awaiter.h>
#include <riz/io/uart_service.h>
#include <riz/timer/timer_service.h>

#include <algorithm>
#include <cassert>

using namespace riz::io;

riz::coro::awaiter::uart_receive_awaiter uart_service::receive(
    void* data, std::size_t len, std::uint32_t timeout_ms) noexcept {
    return {*this, data, len, timeout_ms};
}

riz::coro::awaiter::uart_transmit_awaiter uart_service::transmit(
    const void* data, std::size_t len, std::uint32_t timeout_ms) noexcept {
    return {*this, data, len, timeout_ms};
}

bool uart_service::try_fill_receiver(coro::awaiter::uart_receive_awaiter& receiver) noexcept {
    if (rx_ring_buffer_.size() == 0) {
        return false;
    }
    std::size_t rem = receiver.len_ - receiver.offset_;
    while (std::size_t n = rx_ring_buffer_.pop_front(receiver.data_ + receiver.offset_, rem)) {
        rem -= n;
        receiver.offset_ += n;
    }
    return (rem == 0);
}

std::size_t uart_service::try_drain_transmitter(
    coro::awaiter::uart_transmit_awaiter& transmitter,
    const std::byte*& data, std::size_t chunk_size) noexcept {
    std::size_t rem = transmitter.len_ - transmitter.offset_;
    std::size_t consumed = std::min(rem, chunk_size);
    if (consumed > 0) {
        data = transmitter.data_ + transmitter.offset_;
        transmitter.offset_ += consumed;
    }
    return consumed;
}

void uart_service::run() noexcept {
    process_write();
    process_read();
    process_error();
}

void uart_service::complete_active_read(errcode status) noexcept {
    timer::timer_service::instance().cancel(rx_timer_);
    active_read_awaiter_->on_resume(status);
    active_read_awaiter_ = nullptr;
}

void uart_service::complete_active_write(errcode status) noexcept {
    timer::timer_service::instance().cancel(tx_timer_);
    active_write_awaiter_->on_resume(status);
    active_write_awaiter_ = nullptr;
}

void uart_service::process_write() noexcept {
    if (write_ready_.load(std::memory_order_acquire)) {
        if (active_write_awaiter_ == nullptr) {
            using awaiter = riz::coro::awaiter::uart_transmit_awaiter;
            auto s = static_cast<awaiter*>(dequeue_pending_write());
            if (s != nullptr) {
                active_write_awaiter_ = s;
                if (s->timeout_ms_ > 0) {
                    tx_timer_.self = this;
                    tx_timer_.on_expire = &on_tx_timeout;
                    timer::timer_service::instance().submit_ms(s->timeout_ms_, tx_timer_);
                }
            }
        }
        if (active_write_awaiter_ != nullptr) {
            const std::byte* data = nullptr;
            constexpr std::size_t chunk_size = 64u;
            std::size_t len = try_drain_transmitter(*active_write_awaiter_, data, chunk_size);
            if (len > 0) {
                int rc = dev_.start_transmit(data, len);
                if (rc == 0) {
                    write_ready_.store(false, std::memory_order_release);
                } else {
                    complete_active_write(errcode::unknown_hw_error);
                }
            } else {
                complete_active_write(errcode::success);
            }
        }
    }
}

void uart_service::process_read() noexcept {
    if (active_read_awaiter_ == nullptr) {
        using awaiter = coro::awaiter::uart_receive_awaiter;
        auto* r = static_cast<awaiter*>(dequeue_pending_read());
        if (r != nullptr) {
            active_read_awaiter_ = r;
            if (try_fill_receiver(*r)) {
                r->on_resume(errcode::success);
                active_read_awaiter_ = nullptr;
            } else if (r->timeout_ms_ > 0) {
                rx_timer_.self = this;
                rx_timer_.on_expire = &on_rx_timeout;
                timer::timer_service::instance().submit_ms(r->timeout_ms_, rx_timer_);
            }
        }
    }
    if (active_read_awaiter_ != nullptr) {
        if (try_fill_receiver(*active_read_awaiter_)) {
            complete_active_read(errcode::success);
        }
    }
}

void uart_service::process_error() noexcept {
    errcode status = hw_status_.exchange(errcode::success, std::memory_order_acquire);
    if (status == errcode::success) {
        return;
    }
    if (active_read_awaiter_ != nullptr) {
        complete_active_read(status);
    }
}

void uart_service::on_tx_complete() noexcept {
    write_ready_.store(true, std::memory_order_release);
}

void uart_service::on_tx_timeout(timer::timer_node* tn) noexcept {
    auto* node = static_cast<tx_timeout_node*>(tn);
    auto& self = *node->self;
    if (!self.active_write_awaiter_)
        return;
    self.dev_.abort_transmit();
    self.write_ready_.store(true, std::memory_order_release);
    self.complete_active_write(errcode::timeout);
}

void uart_service::on_rx_timeout(timer::timer_node* tn) noexcept {
    auto* node = static_cast<rx_timeout_node*>(tn);
    auto& self = *node->self;
    if (!self.active_read_awaiter_)
        return;
    self.complete_active_read(errcode::timeout);
}

void uart_service::on_rx_complete(const std::byte* data, std::size_t len) noexcept {
    rx_ring_buffer_.push(data, len);
}

void uart_service::on_rx_idle(const std::byte* data, std::size_t len) noexcept {
    rx_ring_buffer_.push(data, len);
}

void uart_service::on_rx_error(errcode err) noexcept {
    hw_status_.store(err, std::memory_order_release);
}
