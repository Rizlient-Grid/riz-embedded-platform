#pragma once

#include <riz/container/intrusive/fifo_queue.h>
#include <riz/coro/channel/errcode.hpp>

namespace riz::coro::channel {

struct channel_node : container::intrusive::fifo_queue::node_type {
    void (*on_resume)(channel_node*, void* data, channel::errcode) noexcept = nullptr;
};

} // namespace riz::coro::channel
