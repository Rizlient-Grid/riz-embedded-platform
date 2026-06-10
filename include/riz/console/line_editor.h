#pragma once

#include <cstddef>

namespace riz::console {

class line_editor {
public:
    template<std::size_t N>
    line_editor(char (&storage)[N])
        : buffer_ {storage}
        , capacity_ {N} {
        buffer_[0] = '\0';
    }

    bool append(char c) noexcept;
    bool backspace() noexcept;
    char* data() noexcept;
    void clear() noexcept;
    const char* c_str() const noexcept;

private:
    char* buffer_ {nullptr};
    std::size_t capacity_ {0};
    std::size_t pos_ {0};
};

} // namespace riz::console
