#pragma once

#include <riz/coro/promise/schedulable_task_promise.hpp>
#include <riz/timer/timer_node.h>
#include <riz/timer/timer_service.h>

#include <cassert>
#include <coroutine>
#include <cstdint>

namespace riz::coro::awaiter {

class timer_awaiter {
public:
    timer_awaiter(std::uint32_t sleep_duration, bool use_raw_tick = false)
        : sleep_duration_ {sleep_duration}
        , use_raw_tick_ {use_raw_tick} {}

    ~timer_awaiter() {
        auto& tsvr = timer::timer_service::instance();
        tsvr.cancel(timer_node_);
    }

    bool await_ready() const noexcept {
        return sleep_duration_ == 0u;
    }

    template<typename Promise>
        requires constraint::is_schedulable_task_promise_v<Promise>
    void await_suspend(std::coroutine_handle<Promise> handle) noexcept {
        timer_node_.sched_node = &handle.promise().schedulable_node;
        timer_node_.on_expire = &on_expire;
        auto& tsvr = timer::timer_service::instance();
        if (use_raw_tick_) {
            tsvr.submit(sleep_duration_, timer_node_);
        } else {
            tsvr.submit_ms(sleep_duration_, timer_node_);
        }
    }

    void await_resume() noexcept {}

private:
    static void on_expire(timer::timer_node* tn) noexcept {
        auto n = static_cast<node*>(tn);
        auto sched_node = n->sched_node;
        assert(sched_node);
        auto executor = sched_node->executor;
        assert(executor);
        executor->post(*sched_node);
    }

private:
    struct node : timer::timer_node {
        execution::schedulable_node* sched_node;
    };

private:
    std::uint32_t sleep_duration_ {0};
    node timer_node_;
    bool use_raw_tick_ {false};
};

} // namespace riz::coro::awaiter
