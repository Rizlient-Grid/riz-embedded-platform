#include <riz/coro/execution/scheduler.h>

using namespace riz::coro::execution;

void scheduler::post(node_type& entry) noexcept {
    ready_queue_.push(entry);
}

void scheduler::run() noexcept {
    while (fifo_queue::node_type* n = ready_queue_.pop_front()) {
        node_type* tn = static_cast<node_type*>(n);
        if (tn->coro_handle) {
            tn->coro_handle.resume();
        }
    }
}

bool scheduler::run_once() noexcept {
    if (fifo_queue::node_type* n = ready_queue_.pop_front()) {
        node_type* tn = static_cast<node_type*>(n);
        if (tn->coro_handle) {
            tn->coro_handle.resume();
        }
        return true;
    }
    return false;
}
