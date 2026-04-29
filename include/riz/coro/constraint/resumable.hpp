#pragma once

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

} // namespace riz::coro::constraint
