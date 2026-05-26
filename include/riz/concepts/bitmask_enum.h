#pragma once

#include <type_traits>

namespace riz {

template<typename T>
struct enable_bitmask : std::false_type {};

template<typename T>
concept bitmask_enum = std::is_enum_v<T> && enable_bitmask<T>::value;

template<bitmask_enum E>
constexpr E operator|(E a, E b) noexcept {
    using U = std::underlying_type_t<E>;
    return static_cast<E>(static_cast<U>(a) | static_cast<U>(b));
}

template<bitmask_enum E>
constexpr E& operator|=(E& a, E b) noexcept {
    return a = a | b;
}

template<bitmask_enum E>
constexpr bool operator&(E a, E b) noexcept {
    using U = std::underlying_type_t<E>;
    return (static_cast<U>(a) & static_cast<U>(b)) != 0;
}

} // namespace riz
