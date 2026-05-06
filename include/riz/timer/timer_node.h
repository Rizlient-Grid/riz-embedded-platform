#pragma once

#include <riz/container/intrusive/delta_queue.h>

namespace riz::timer {

struct timer_node : container::intrusive::delta_queue::node_type {
    void (*on_expire)(timer_node*) noexcept = nullptr;
};

} // namespace riz::timer
