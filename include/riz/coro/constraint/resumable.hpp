#pragma once

namespace riz::coro::promise {
template<typename T>
class schedulable_task_promise;
} // namespace riz::coro::promise

namespace riz::coro::constraint {

template<typename T>
concept resumable = requires {
    typename T::promise_type;
    requires T::tag_is_resumable;
};

template<typename T>
concept resumable_pair = requires {
    typename T::resumable_type;
    typename T::promise_type;
};

template<typename>
inline constexpr bool is_schedulable_task_promise_v = false;
template<typename T>
inline constexpr bool is_schedulable_task_promise_v<promise::schedulable_task_promise<T>> = true;

} // namespace riz::coro::constraint
