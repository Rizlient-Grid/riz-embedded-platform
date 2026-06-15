#pragma once

#include "argument_parser.h"
#include "line_editor.h"

#include <riz/constraints.h>
#include <riz/container/byte_ring_buffer.h>
#include <riz/coro/resumable/schedulable_task.hpp>
#include <riz/io/uart_service.h>

namespace riz::console {

using command_handler = coro::resumable::schedulable_task<void> (*)(
    coro::execution::scheduler& sched, int argc, const char* const* argv);

struct command_entry {
    const char* name;
    command_handler handler;
};

class console : public immovable {
public:
    template<std::size_t CmdCount, std::size_t LineSize, std::size_t ArgvSize,
        std::size_t WriteBufferSize>
    console(io::uart_service& serial, command_entry (&commands)[CmdCount],
        char (&line_buf)[LineSize], const char* (&argv_buf)[ArgvSize],
        std::byte (&WriteBuffer)[WriteBufferSize])
        : serial_ {serial}
        , editor_ {line_buf}
        , parser_ {argv_buf, ArgvSize}
        , write_buffer_ {WriteBuffer}
        , commands_ {commands}
        , max_commands_ {CmdCount} {}

public:
    coro::resumable::schedulable_task<void> run(coro::execution::scheduler& sched);
    int print(const char* fmt, ...);

private:
    coro::resumable::schedulable_task<void> dispatch(
        coro::execution::scheduler& sched, int argc, const char** argv);

private:
    io::uart_service& serial_;
    line_editor editor_;
    argument_parser parser_;
    container::byte_ring_buffer write_buffer_;
    command_entry* commands_ {nullptr};
    std::size_t max_commands_ {0};
};

} // namespace riz::console
