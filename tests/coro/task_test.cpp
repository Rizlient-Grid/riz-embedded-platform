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
    std::cout << "here" << std::endl;
    co_return 0;
}

riz::coro::task<int, void> simple_coro2()
{
    int rc = co_await simple_coro();
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    co_return rc;
}

} // namespace

TEST(TaskTest, ConstructsFromCoroutine)
{
    auto t = simple_coro();
    t.resume();
    EXPECT_EQ(t.get_handle().done(), true);
}

TEST(TaskTest, CoawaitAnotherTask)
{
    auto coro = []() -> riz::coro::task<int, void> {
        int rc = co_await simple_coro();
        std::cout << "here2" << std::endl;
        co_return rc;
    };

    auto task = coro();
    task.resume();
    EXPECT_EQ(task.get_handle().done(), true);
}

TEST(TaskTest, CoawaitChainedTasks)
{
    auto coro = []() -> riz::coro::task<int, void> {
        int rc = co_await simple_coro2();
        std::cout << "here2" << std::endl;
        co_return rc;
    };

    auto task = coro();
    task.resume();
    EXPECT_EQ(task.get_handle().done(), true);
}