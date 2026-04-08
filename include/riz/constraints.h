#pragma once

namespace riz {

struct noncopyable
{
    noncopyable() = default;
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
};

} // namespace riz
