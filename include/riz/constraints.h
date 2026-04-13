#pragma once

namespace riz {

struct noncopyable {
    noncopyable() = default;
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
};

struct moveonly {
    moveonly() = default;
    moveonly(moveonly&&) = default;
    moveonly(const moveonly&) = delete;
    moveonly& operator=(moveonly&&) = default;
    moveonly& operator=(const moveonly&) = delete;
};

} // namespace riz
