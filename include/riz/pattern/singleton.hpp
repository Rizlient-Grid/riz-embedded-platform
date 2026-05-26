#pragma once

#include <riz/constraints.h>

#include <type_traits>

namespace riz::pattern {

template<typename T>
class singleton : public immovable {
public:
    static T& instance() {
        static T obj;
        return obj;
    }

protected:
    singleton() = default;
};

} // namespace riz::pattern