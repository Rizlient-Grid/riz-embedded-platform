#pragma once

#include <riz/constraints.h>
#include <riz/container/intrusive/fifo_queue.h>

#include <coroutine>
#include <type_traits>

namespace riz::coro {

template<typename Mutex = void, typename Sem = void>
class scheduler : public noncopyable {
    struct empty_mutex {};
    struct empty_sem {};
    using fifo_queue = riz::container::intrusive::fifo_queue;
    using mutex_type = std::conditional_t<std::is_void_v<Mutex>, empty_mutex, Mutex>;
    using sem_type = std::conditional_t<std::is_void_v<Sem>, empty_sem, Sem>;;
public:
    struct node_type: fifo_queue::node_type {
        std::coroutine_handle<> coro_handle;
    };

    scheduler() = default;

    void post(node_type& entry)
    {
        ready_queue_.push(entry);
    }

    void run()
    {
        while (fifo_queue::node_type* n = ready_queue_.pop_front()) {
            node_type* tn = static_cast<node_type*>(n);
            if (tn->coro_handle) {
                tn->coro_handle.resume();
            }
        }
    }

    bool run_once()
    {
        if (fifo_queue::node_type* n = ready_queue_.pop_front()) {
            node_type* tn = static_cast<node_type*>(n);
            if (tn->coro_handle) {
                tn->coro_handle.resume();
            }
            return true;
        }
        return false;
    }

private:
    fifo_queue ready_queue_;
    [[no_unique_address]] mutex_type mutex_;
    [[no_unique_address]] sem_type sem_;
};

template<typename T>
class schedulable_task;

template<typename T>
[[nodiscard]] schedulable_task<T> start(schedulable_task<T>&& task)
{
    auto& promise = task.handle().promise();
    auto& scheduler = promise.scheduler;
    auto& schedulable_node = promise.schedulable_node;
    assert(!promise.started_);
    scheduler.post(schedulable_node);
    return std::move(task);
}

} // namespace riz::coro