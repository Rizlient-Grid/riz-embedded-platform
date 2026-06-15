#include <riz/console/line_editor.h>

using namespace riz::console;

bool line_editor::append(char c) noexcept {
    if (pos_ >= (capacity_ - 1)) { return false; }
    buffer_[pos_++] = c;
    buffer_[pos_] = '\0';
    return true;
}

bool line_editor::backspace() noexcept {
    if (pos_ == 0) { return false; }
    buffer_[--pos_] = '\0';
    return true;
}

char* line_editor::data() noexcept {
    return buffer_;
}

void line_editor::clear() noexcept {
    pos_ = 0;
    buffer_[0] = '\0';
}

const char* line_editor::c_str() const noexcept {
    return buffer_;
}
