#include <riz/console/console.h>
#include <riz/console/logger.h>

#include <riz/coro/awaiter/yield_awaiter.h>
#include <riz/coro/sleep.h>
#include <riz/io/const.h>

#include <algorithm>
#include <cstdarg>
#include <cstddef>
#include <cstdio>
#include <cstring>

using namespace riz::console;

riz::coro::resumable::schedulable_task<void> console::run(coro::execution::scheduler& sched) {
    serial_.start();
    for (;;) {
        char c;
        errcode rc = co_await serial_.receive(&c, sizeof(c));
        if (rc == errcode::success) {
            if (c == '\r' || c == '\n') {
                char* line = editor_.data();
                int argc = parser_.parse(line);
                if (argc > 0) {
                    co_await dispatch(sched, parser_.argc(), parser_.argv());
                }
                editor_.clear();
            } else if (c == '\b') {
                editor_.backspace();
            } else {
                editor_.append(c);
            }
        }
        std::byte tx_chunk[64];
        while (std::size_t n = write_buffer_.pop_front(tx_chunk, sizeof(tx_chunk))) {
            co_await serial_.transmit(tx_chunk, n, riz::io::wait_forever);
        }
        co_await coro::awaiter::yield_awaiter {};
    }
}

int console::print(const char* fmt, ...) {
    char buff[64];
    va_list ap;
    va_start(ap, fmt);
    int rc = std::vsnprintf(buff, sizeof(buff), fmt, ap);
    if (rc > 0) {
        int len = std::min(static_cast<std::size_t>(rc), sizeof(buff) - 1);
        write_buffer_.push(reinterpret_cast<std::byte*>(buff), len);
    }
    va_end(ap);
    return rc;
}

riz::coro::resumable::schedulable_task<void> console::dispatch(
    coro::execution::scheduler& sched, int argc, const char** argv) {
    for (std::size_t i = 0; i < max_commands_; ++i) {
        if (strcmp(commands_[i].name, argv[0]) == 0) {
        	INFO("Command: %s", argv[0]);
            co_await commands_[i].handler(sched, argc, argv);
            co_return;
        }
    }
    INFO("Unknown: %s", argv[0]);
}
