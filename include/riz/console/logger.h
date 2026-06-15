#pragma once

#include "console.h"

#include <riz/pattern/singleton.hpp>

#define TRACE(...) LOG_IMPL("T", __VA_ARGS__)
#define INFO(...) LOG_IMPL("I", __VA_ARGS__)
#define ERROR(...) LOG_IMPL("E", __VA_ARGS__)
#define LOG_IMPL(tag, fmt, ...)                                                                    \
    do {                                                                                           \
        riz::console::logger::instance().log(tag " " fmt "\n\r" __VA_OPT__(, ) __VA_ARGS__);       \
    } while (0)

namespace riz::console {

class logger : public pattern::singleton<logger> {
public:
    logger() = default;

    void init(console& c) noexcept {
        console_ = &c;
    }

    template<typename... Ts>
    void log(const char* fmt, Ts&&... ts) noexcept {
        console_->print(fmt, ts...);
    }

private:
    console* console_ {nullptr};

private:
    friend class pattern::singleton<logger>;
};

} // namespace riz::console
