#pragma once

#include <riz/coro/execution/scheduler.h>
#include <riz/coro/resumable/schedulable_task.hpp>

namespace riz::coro::execution {

template<typename T>
[[nodiscard]] resumable::schedulable_task<T> start(resumable::schedulable_task<T>&& task) {
    auto& promise = task.promise();
    auto& schedulable_node = promise.schedulable_node;
    assert(schedulable_node.executor);
    auto& executor = *schedulable_node.executor;
    assert(!promise.started);
    executor.post(schedulable_node);
    return std::move(task);
}

} // namespace riz::coro::execution
