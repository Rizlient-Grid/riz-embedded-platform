#pragma once

#include <cstddef>

namespace riz::console {

class argument_parser {
public:
    argument_parser(const char** argv_storage, std::size_t max_args)
        : argv_ {argv_storage}
        , max_args_ {max_args} {}
    
    int parse(char* line) noexcept;

    int argc() const noexcept;
    const char** const argv() const noexcept;

private:
    const char** argv_ {nullptr};
    int argc_ {0};
    const std::size_t max_args_ {0};
};

} // riz::console
