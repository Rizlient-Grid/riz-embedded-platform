#include <gtest/gtest.h>

#include <riz/coro/schedulable_task.hpp>
#include <riz/coro/scheduler.hpp>
#include <riz/coro/task.hpp>

#include <coroutine>
#include <type_traits>
#include <utility>

using namespace riz::coro;

// ---------------------------------------------------------------------------
// Static type trait tests
// ---------------------------------------------------------------------------

TEST(SchedulableTaskTest, HasCorrectPromiseType)
{
    static_assert(
        std::is_same_v<schedulable_task<int>::promise_type,
                       schedulable_task_promise<int>>);
    static_assert(
        std::is_same_v<schedulable_task<void>::promise_type,
                       schedulable_task_promise<void>>);
}

TEST(SchedulableTaskTest, HasCorrectReturnType)
{
    static_assert(std::is_same_v<schedulable_task<int>::return_type, int>);
    static_assert(std::is_same_v<schedulable_task<void>::return_type, void>);
    static_assert(std::is_same_v<schedulable_task<double>::return_type, double>);
}

TEST(SchedulableTaskTest, HasCorrectSchedulerType)
{
    static_assert(std::is_same_v<schedulable_task<int>::scheduler_type, scheduler<>>);
}

TEST(SchedulableTaskTest, TagIsResumable)
{
    static_assert(schedulable_task<int>::tag_is_resumable);
    static_assert(schedulable_task<void>::tag_is_resumable);
}

TEST(SchedulableTaskTest, SatisfiesResumableConcept)
{
    static_assert(Resumable<schedulable_task<int>>);
    static_assert(Resumable<schedulable_task<void>>);
    static_assert(Resumable<schedulable_task<double>>);
}

TEST(SchedulableTaskTest, MoveSemantics)
{
    using T = schedulable_task<int>;
    static_assert(!std::is_copy_constructible_v<T>);
    static_assert(!std::is_copy_assignable_v<T>);
    static_assert(std::is_move_constructible_v<T>);
    static_assert(std::is_move_assignable_v<T>);
}

TEST(SchedulableTaskTest, SizeIsOnePointer)
{
    // schedulable_task privately inherits resumable, which holds a single
    // coroutine_handle (one pointer in size).
    EXPECT_EQ(sizeof(schedulable_task<int>), sizeof(void*));
}

// ---------------------------------------------------------------------------
// Scheduler type trait tests
// ---------------------------------------------------------------------------

TEST(SchedulerTest, IsNoncopyable)
{
    static_assert(!std::is_copy_constructible_v<scheduler<>>);
    static_assert(!std::is_copy_assignable_v<scheduler<>>);
}

TEST(SchedulerTest, IsDefaultConstructible)
{
    static_assert(std::is_default_constructible_v<scheduler<>>);
}

TEST(SchedulerTest, NodeTypeHasCoroHandle)
{
    scheduler<>::node_type node;
    // node_type extends fifo_queue::node_type (has `next`), plus coro_handle
    (void)node.next;
    (void)node.coro_handle;
}

// ---------------------------------------------------------------------------
// Basic scheduling: start() → run() → done()
// ---------------------------------------------------------------------------

namespace {

schedulable_task<int> simple_int_task(scheduler<>& sched)
{
    co_return 42;
}

schedulable_task<void> simple_void_task(scheduler<>& sched)
{
    co_return;
}

schedulable_task<int> zero_task(scheduler<>& sched)
{
    co_return 0;
}

} // namespace

TEST(SchedulableTaskTest, BasicIntTask_ScheduledToCompletion)
{
    scheduler<> sched;
    auto task = start(simple_int_task(sched));
    sched.run();
    EXPECT_TRUE(task.done());
    EXPECT_EQ(task.take_result(), 42);
}

TEST(SchedulableTaskTest, BasicVoidTask_ScheduledToCompletion)
{
    scheduler<> sched;
    auto task = start(simple_void_task(sched));
    sched.run();
    EXPECT_TRUE(task.done());
}

TEST(SchedulableTaskTest, DoneIsFalseBeforeRun)
{
    scheduler<> sched;
    auto task = start(zero_task(sched));
    // Task has been posted but not yet executed by scheduler
    EXPECT_FALSE(task.done());
    sched.run();
    EXPECT_TRUE(task.done());
}

// ---------------------------------------------------------------------------
// Multiple tasks — FIFO ordering
// ---------------------------------------------------------------------------

namespace {

// Records the order in which coroutines execute
schedulable_task<void> ordered_task(scheduler<>& sched, int id, std::vector<int>& log)
{
    log.push_back(id);
    co_return;
}

} // namespace

TEST(SchedulerTest, MultipleTasks_FIFOOrder)
{
    scheduler<> sched;
    std::vector<int> log;

    auto t1 = start(ordered_task(sched, 1, log));
    auto t2 = start(ordered_task(sched, 2, log));
    auto t3 = start(ordered_task(sched, 3, log));

    sched.run();

    ASSERT_EQ(log.size(), 3u);
    EXPECT_EQ(log[0], 1);
    EXPECT_EQ(log[1], 2);
    EXPECT_EQ(log[2], 3);
}

// ---------------------------------------------------------------------------
// run_once() semantics
// ---------------------------------------------------------------------------

TEST(SchedulerTest, RunOnce_ExecutesSingleTask)
{
    scheduler<> sched;
    std::vector<int> log;

    auto t1 = start(ordered_task(sched, 1, log));
    auto t2 = start(ordered_task(sched, 2, log));

    EXPECT_TRUE(sched.run_once());
    ASSERT_EQ(log.size(), 1u);
    EXPECT_EQ(log[0], 1);

    EXPECT_TRUE(sched.run_once());
    ASSERT_EQ(log.size(), 2u);
    EXPECT_EQ(log[1], 2);

    // Queue is now empty
    EXPECT_FALSE(sched.run_once());
}

TEST(SchedulerTest, RunOnceReturnsFalseOnEmptyQueue)
{
    scheduler<> sched;
    EXPECT_FALSE(sched.run_once());
}

TEST(SchedulerTest, RunOnEmptyQueueIsNoOp)
{
    scheduler<> sched;
    // Should not crash or hang
    sched.run();
}

// ---------------------------------------------------------------------------
// co_await nested schedulable_task
// ---------------------------------------------------------------------------

namespace {

schedulable_task<int> inner_task(scheduler<>& sched)
{
    co_return 10;
}

schedulable_task<int> outer_task(scheduler<>& sched)
{
    int val = co_await inner_task(sched);
    co_return val + 5;
}

} // namespace

TEST(SchedulableTaskTest, CoawaitNestedTask)
{
    scheduler<> sched;
    auto task = start(outer_task(sched));
    sched.run();
    EXPECT_TRUE(task.done());
    EXPECT_EQ(task.take_result(), 15);
}

// ---------------------------------------------------------------------------
// co_await chained schedulable_tasks (deeper nesting)
// ---------------------------------------------------------------------------

namespace {

schedulable_task<int> chain_a(scheduler<>& sched)
{
    co_return 1;
}

schedulable_task<int> chain_b(scheduler<>& sched)
{
    int a = co_await chain_a(sched);
    co_return a + 2;
}

schedulable_task<int> chain_c(scheduler<>& sched)
{
    int b = co_await chain_b(sched);
    co_return b + 3;
}

} // namespace

TEST(SchedulableTaskTest, CoawaitChainedTasks)
{
    scheduler<> sched;
    auto task = start(chain_c(sched));
    sched.run();
    EXPECT_TRUE(task.done());
    EXPECT_EQ(task.take_result(), 6); // 1 + 2 + 3
}

// ---------------------------------------------------------------------------
// Task returning reference / pointer types
// ---------------------------------------------------------------------------

namespace {

schedulable_task<int*> pointer_task(scheduler<>& sched, int* ptr)
{
    co_return ptr;
}

} // namespace

TEST(SchedulableTaskTest, PointerReturnType)
{
    scheduler<> sched;
    int value = 99;
    auto task = start(pointer_task(sched, &value));
    sched.run();
    EXPECT_TRUE(task.done());
    EXPECT_EQ(task.take_result(), &value);
}

// ---------------------------------------------------------------------------
// schedulable_task can co_await a plain task<T>
// ---------------------------------------------------------------------------

namespace {

riz::coro::task<int> plain_task()
{
    co_return 7;
}

schedulable_task<int> schedulable_with_plain_task(scheduler<>& sched)
{
    int val = co_await plain_task();
    co_return val * 3;
}

} // namespace

TEST(SchedulableTaskTest, CoawaitPlainTask)
{
    scheduler<> sched;
    auto task = start(schedulable_with_plain_task(sched));
    sched.run();
    EXPECT_TRUE(task.done());
    EXPECT_EQ(task.take_result(), 21);
}

// ---------------------------------------------------------------------------
// Scheduler with multiple independent tasks
// ---------------------------------------------------------------------------

namespace {

schedulable_task<int> accumulating_task(scheduler<>& sched, int base)
{
    co_return base * 2;
}

} // namespace

TEST(SchedulerTest, MultipleIndependentTasks)
{
    scheduler<> sched;

    auto t1 = start(accumulating_task(sched, 1));
    auto t2 = start(accumulating_task(sched, 2));
    auto t3 = start(accumulating_task(sched, 3));

    sched.run();

    EXPECT_TRUE(t1.done());
    EXPECT_TRUE(t2.done());
    EXPECT_TRUE(t3.done());

    EXPECT_EQ(t1.take_result(), 2);
    EXPECT_EQ(t2.take_result(), 4);
    EXPECT_EQ(t3.take_result(), 6);
}

// ---------------------------------------------------------------------------
// run_once interleaved with new posts
// ---------------------------------------------------------------------------

TEST(SchedulerTest, RunOnceInterleavedWithPosts)
{
    scheduler<> sched;
    std::vector<int> log;

    auto t1 = start(ordered_task(sched, 1, log));
    sched.run_once();
    ASSERT_EQ(log.size(), 1u);
    EXPECT_EQ(log[0], 1);

    // Post another task after first run_once
    auto t2 = start(ordered_task(sched, 2, log));
    sched.run_once();
    ASSERT_EQ(log.size(), 2u);
    EXPECT_EQ(log[1], 2);

    EXPECT_FALSE(sched.run_once());
}
