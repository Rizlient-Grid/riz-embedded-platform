#pragma once

#include <riz/container/intrusive/delta_queue.h>
#include <riz/hal/tick_source.h>
#include <riz/pattern/singleton.hpp>
#include <riz/timer/timer_node.h>

#include <cstdint>

namespace riz::timer {

class timer_service : public pattern::singleton<timer_service> {
    friend class pattern::singleton<timer_service>;

public:
    void submit(std::uint32_t duration, timer_node& node) noexcept;
    void submit_ms(std::uint32_t duration, timer_node& node) noexcept;
    void cancel(timer_node& node) noexcept;
    void run() noexcept;

private:
    timer_service();

private:
    [[no_unique_address]] hal::tick_source tick_source_;
    container::intrusive::delta_queue delta_queue_;
    std::uint32_t prev_ts_tick;
};

} // namespace riz::timer
