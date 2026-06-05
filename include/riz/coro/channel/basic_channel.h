#pragma once

#include <riz/constraints.h>
#include <riz/container/ring_queue.hpp>
#include <riz/coro/awaiter/basic_channel_read_awaiter.h>
#include <riz/coro/awaiter/basic_channel_write_awaiter.h>
#include <riz/coro/transport/read_acceptor.h>
#include <riz/coro/transport/write_acceptor.h>

#include <type_traits>

namespace riz::coro::channel {

class basic_channel
    : public immovable
    , public transport::read_acceptor
    , public transport::write_acceptor {
public:
    template<typename T, std::size_t Capacity>
        requires (std::is_trivially_copyable_v<T>)
    basic_channel(T (&storage)[Capacity])
        : buffer_ {storage, sizeof(T), Capacity}
        , entry_size_ {sizeof(T)} {}

    explicit basic_channel(std::size_t entry_size)
        : entry_size_ {entry_size} {}

    std::size_t capacity() const noexcept {
        return buffer_.capacity();
    }

    errcode try_write(const void* value) noexcept;
    errcode try_read(void* value) noexcept;
    void close() noexcept;

    [[nodiscard]] bool closed() const noexcept {
        return closed_;
    }

private:
    container::raw_ring_queue buffer_;
    const std::size_t entry_size_;
    bool closed_ {false};

private:
    friend awaiter::basic_channel_read_awaiter;
    friend awaiter::basic_channel_write_awaiter;
};

} // namespace riz::coro::channel
