#pragma once

#include <riz/constraints.h>

#include <type_traits>

namespace riz::pattern {

template<typename T>
class singleton : public immovable {
public:
    static T& instance() {
        static_assert(!riz::is_public_default_constructible<T>());
        static T obj;
        return obj;
    }

protected:
    singleton() = default;
};

} // namespace riz::pattern