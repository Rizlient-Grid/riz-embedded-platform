#pragma once

namespace riz {

struct noncopyable {
    noncopyable() = default;
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
};

struct nonmovable {
    nonmovable() = default;
    nonmovable(const nonmovable&) = default;
    nonmovable(nonmovable&&) = delete;
    nonmovable& operator=(const nonmovable&) = default;
    nonmovable& operator=(nonmovable&&) = delete;
};

struct moveonly {
    moveonly() = default;
    moveonly(moveonly&&) = default;
    moveonly(const moveonly&) = delete;
    moveonly& operator=(moveonly&&) = default;
    moveonly& operator=(const moveonly&) = delete;
};

struct immovable : noncopyable, nonmovable {};

template<typename T>
concept accessible_default_constructible = requires { T {}; };

template<typename T>
consteval bool has_public_default_constructible()
{
    return accessible_default_constructible<T>;
}

} // namespace riz
