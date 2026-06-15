#include <riz/console/argument_parser.h>

using namespace riz::console;

int argument_parser::parse(char* line) noexcept {
    argc_ = 0;
    if (!line) {
        return 0;
    }
    char* p = line;
    while (*p != '\0') {
        while (*p == ' ' || *p == '\t') {
            ++p;
        }
        if (*p == '\0') {
            break;
        }
        if (static_cast<std::size_t>(argc_) >= max_args_) {
            break;
        }
        argv_[argc_++] = p;
        while (*p != '\0' && *p != ' ' && *p != '\t') {
            ++p;
        }
        if (*p != '\0') {
            *(p++) = '\0';
        }
    }
    return argc_;
}

int argument_parser::argc() const noexcept {
    return argc_;
}

const char** const argument_parser::argv() const noexcept {
    return argv_;
}