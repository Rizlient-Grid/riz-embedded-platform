#include <riz/io/uart_service.h>
#include <riz/coro/awaiter/uart_receive_awaiter.h>
#include <riz/coro/awaiter/uart_transmit_awaiter.h>
#include <riz/timer/timer_service.h>

#include <cassert>

riz::coro::awaiter::uart_receive_awaiter riz::io::uart_service::receive(void* data, std::size_t len, std::uint32_t timeout_ms) noexcept {
    return {*this, data, len, timeout_ms};
}

riz::coro::awaiter::uart_transmit_awaiter riz::io::uart_service::transmit(const void* data, std::size_t len, std::uint32_t timeout_ms) noexcept {
    return {*this, data, len, timeout_ms};
}

bool riz::io::uart_service::try_fill_receiver(riz::coro::awaiter::uart_receive_awaiter& receiver) noexcept {
    if (rx_ring_buffer_.size() ==0) {
        return false;
    }
    std::size_t rem = receiver.len_ - receiver.offset_;
    while (std::size_t n = rx_ring_buffer_.pop_front(receiver.data_ + receiver.offset_, rem)) {
        rem -= n;
        receiver.offset_ += n;
    }
    return (rem == 0);
}

void riz::io::uart_service::push_pending_sender(container::intrusive::fifo_queue::node& node) noexcept {
    pending_senders_.push(node);
}

void riz::io::uart_service::push_pending_receiver(container::intrusive::fifo_queue::node& node) noexcept {
    pending_receivers_.push(node);
}

void riz::io::uart_service::run() noexcept {
    if (tx_ready_.load(std::memory_order_acquire)) {
        if (active_sender_awaiter_ == nullptr) {
            using awaiter = riz::coro::awaiter::uart_transmit_awaiter;
            auto s = static_cast<awaiter*>(pending_senders_.pop_front());
            if (s != nullptr) {
                active_sender_awaiter_ = s;
                if (s->timeout_ms_ > 0) {
                    tx_timer_.self = this;
                    tx_timer_.on_expire = &on_tx_timeout;
                    timer::timer_service::instance().submit_ms(s->timeout_ms_, tx_timer_);
                }
            }
        }
        if (active_sender_awaiter_ != nullptr) {
            const std::byte* data = nullptr;
            constexpr std::size_t chunk_size = 64u;
            std::size_t len = active_sender_awaiter_->consume(data, chunk_size);
            if (len > 0) {
                int rc = dev_.start_transmit(data, len);
                if (rc == 0) {
                    tx_ready_.store(false, std::memory_order_release);
                } else {
                    timer::timer_service::instance().cancel(tx_timer_);
                    active_sender_awaiter_->on_resume(io::errcode::hw_error);
                    active_sender_awaiter_ = nullptr;
                }
            } else {
                timer::timer_service::instance().cancel(tx_timer_);
                active_sender_awaiter_->on_resume(io::errcode::success);
                active_sender_awaiter_ = nullptr;
            }
        }
    }

    if (active_receiver_awaiter_ == nullptr) {
        using awaiter = riz::coro::awaiter::uart_receive_awaiter;
        auto* r = static_cast<awaiter*>(pending_receivers_.pop_front());
        if (r != nullptr) {
            active_receiver_awaiter_ = r;
            if (try_fill_receiver(*r)) {
                r->on_resume(io::errcode::success);
                active_receiver_awaiter_ = nullptr;
            }
        }
    }
    if (active_receiver_awaiter_ != nullptr) {
        if (try_fill_receiver(*active_receiver_awaiter_)) {
            active_receiver_awaiter_->on_resume(io::errcode::success);
            active_receiver_awaiter_ = nullptr;
        }
    }

    hal::errcode status = hw_status_.load(std::memory_order_acquire);
    if (status != hal::errcode::ok) {
        if (active_receiver_awaiter_ != nullptr) {
            active_receiver_awaiter_->on_resume(io::errcode::hw_error);
            active_receiver_awaiter_ = nullptr;
        }
        hw_status_.store(hal::errcode::ok, std::memory_order_release);
    }
}

void riz::io::uart_service::on_tx_complete() noexcept {
    tx_ready_.store(true, std::memory_order_release);
}

void riz::io::uart_service::on_tx_timeout(timer::timer_node* tn) noexcept {
    auto* node = static_cast<tx_timeout_node*>(tn);
    auto& self = *node->self;
    if (!self.active_sender_awaiter_) return;
    self.dev_.abort_transmit();
    self.tx_ready_.store(true, std::memory_order_release);
    self.active_sender_awaiter_->on_resume(io::errcode::timeout);
    self.active_sender_awaiter_ = nullptr;
}

void riz::io::uart_service::on_rx_complete(const std::byte* data, std::size_t len) noexcept {
    rx_ring_buffer_.push(data, len);
}

void riz::io::uart_service::on_rx_idle(const std::byte* data, std::size_t len) noexcept {
    rx_ring_buffer_.push(data, len);
}

void riz::io::uart_service::on_rx_error(riz::hal::errcode err) noexcept {
    hw_status_.store(err, std::memory_order_release);
}
