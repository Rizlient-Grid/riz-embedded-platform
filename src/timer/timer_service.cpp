#include <riz/timer/timer_service.h>

#include <cassert>

using namespace riz::timer;

timer_service::timer_service()
    : prev_ts_tick {tick_source_.now()} {}

void timer_service::submit(std::uint32_t duration, timer_node& node) noexcept {
    assert(node.on_expire);
    delta_queue_.insert(duration, node);
}

void timer_service::submit_ms(
    std::uint32_t duration, timer_node& node) noexcept {
    assert(node.on_expire);
    std::uint32_t ticks = static_cast<std::uint32_t>(
        static_cast<std::uint64_t>(duration) * tick_source_.freq() / 1000u);
    submit(ticks, node);
}

void timer_service::cancel(timer_node& node) noexcept {
    delta_queue_.erase(node);
}

void timer_service::run() noexcept {
    const std::uint32_t now = tick_source_.now();
    std::uint32_t elapsed = now - prev_ts_tick;
    delta_queue_.advance(
        elapsed, [](container::intrusive::delta_queue::node* node) {
            auto tn = static_cast<timer_node*>(node);
            tn->on_expire(tn);
        });
    prev_ts_tick = now;
}
