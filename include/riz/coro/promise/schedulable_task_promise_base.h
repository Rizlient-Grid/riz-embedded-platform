#pragma once

#include <riz/coro/execution/schedulable_node.h>

namespace riz::coro::promise {

struct schedulable_task_promise_base {
    execution::schedulable_node schedulable_node;
};

} // riz::coro::promise
