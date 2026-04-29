#pragma once

#include <riz/coro/execution/scheduler.h>
#include <riz/coro/resumable/schedulable_task.hpp>

namespace riz::coro::execution {

template<typename T>
[[nodiscard]] resumable::schedulable_task<T>
start(resumable::schedulable_task<T>&& task) {
    auto& promise = task.promise();
    auto& executor = promise.executor;
    auto& schedulable_node = promise.schedulable_node;
    assert(!promise.started_);
    executor.post(schedulable_node);
    return std::move(task);
}

} // namespace riz::coro::execution
