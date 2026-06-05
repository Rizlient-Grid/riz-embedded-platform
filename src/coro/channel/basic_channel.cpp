#include <riz/coro/channel/basic_channel.h>

#include <cstring>

using namespace riz::coro::channel;

riz::errcode basic_channel::try_write(const void* value) noexcept {
    if (closed_) {
        return errcode::closed;
    }
    if (auto n = dequeue_pending_read()) {
        auto dst = static_cast<awaiter::basic_channel_read_awaiter*>(n)->value_;
        std::memcpy(dst, value, entry_size_);
        n->on_resume(errcode::success);
        return errcode::success;
    }
    if (buffer_.capacity() > 0) {
        if (buffer_.full()) {
            return errcode::full;
        }
        buffer_.push(value);
        return errcode::success;
    }
    return errcode::full;
}

riz::errcode basic_channel::try_read(void* value) noexcept {
    if (buffer_.capacity() > 0) {
        if (buffer_.pop_front(value)) {
            if (auto n = dequeue_pending_write()) {
                n->on_resume(errcode::success);
            }
            return errcode::success;
        }
    }
    if (closed_) {
        return errcode::closed;
    }
    if (auto n = dequeue_pending_write()) {
        auto src = static_cast<awaiter::basic_channel_write_awaiter*>(n)->value_;
        std::memcpy(value, src, entry_size_);
        n->on_resume(errcode::success);
        return errcode::success;
    }
    return errcode::empty;
}

void basic_channel::close() noexcept {
    closed_ = true;
    while (auto n = dequeue_pending_write()) {
        n->on_resume(errcode::canceled);
    }
    while (auto n = dequeue_pending_read()) {
        n->on_resume(errcode::canceled);
    }
}