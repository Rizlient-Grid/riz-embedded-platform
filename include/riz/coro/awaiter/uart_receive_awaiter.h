#pragma once

#include "schedulable_awaiter_node.h"

#include <coroutine>
#include <cstddef>

namespace riz::io {
class uart_service;
} // namespace riz::io

namespace riz::coro::awaiter {

class uart_receive_awaiter : public schedulable_awaiter_node {
public:
    uart_receive_awaiter(
        io::uart_service& svc, void* data, std::size_t len, std::uint32_t timeout_ms = 0)
        : svc_(svc)
        , data_ {static_cast<std::byte*>(data)}
        , len_ {len}
        , timeout_ms_ {timeout_ms} {}

    bool await_ready() noexcept;
    void await_suspend(std::coroutine_handle<> handle) noexcept;
    errcode await_resume() noexcept;

private:
    io::uart_service& svc_;
    std::byte* data_ {nullptr};
    std::size_t len_ {0};
    std::size_t offset_ {0};
    std::uint32_t timeout_ms_ {0};

private:
    friend class io::uart_service;
};

} // namespace riz::coro::awaiter
