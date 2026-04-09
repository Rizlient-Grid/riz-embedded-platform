#include <gtest/gtest.h>

#include <riz/coro/task.hpp>
#include <riz/coro/promise.hpp>

#include <coroutine>
#include <type_traits>

TEST(TaskTest, HasCorrectPromiseType)
{
    static_assert(
        std::is_same_v<
            riz::coro::task<int, void>::promise_type,
            riz::coro::promise<int, void>>);
}

TEST(TaskTest, SizeIsOnePointer)
{
    EXPECT_EQ(sizeof(riz::coro::task<int, void>), sizeof(void*));
}

TEST(TaskTest, MoveSemantics)
{
    using T = riz::coro::task<int, void>;
    static_assert(!std::is_copy_constructible_v<T>);
    static_assert(!std::is_copy_assignable_v<T>);
    static_assert(!std::is_move_constructible_v<T>);
    static_assert(!std::is_move_assignable_v<T>);
}

namespace {

riz::coro::task<int, void> simple_coro()
{
    co_return 0;
}

} // namespace

TEST(TaskTest, ConstructsFromCoroutine)
{
    auto t = simple_coro();
    t.resume();
    EXPECT_EQ(t.get_handle().done(), true);
}
