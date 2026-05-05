#include <gtest/gtest.h>

#include <riz/coro/execution/execute.hpp>
#include <riz/coro/execution/scheduler.h>
#include <riz/coro/resumable/schedulable_task.hpp>
#include <riz/coro/sleep.h>
#include <riz/timer/timer_service.h>

#include "mock_tick_source.h"

#include <chrono>
#include <cstdint>
#include <vector>

using namespace riz::coro;

namespace {

riz::timer::timer_service& ts() {
    return riz::timer::timer_service::instance();
}

} // namespace

class TimerAwaiterTest : public ::testing::Test {
protected:
    void SetUp() override {
        riz_mock_tick_set(0);
        ts().run();
    }
};

TEST_F(TimerAwaiterTest, SleepZeroDoesNotSuspend) {
    execution::scheduler sched;
    bool reached = false;

    auto task = [](execution::scheduler& sched,
                    bool& flag) -> resumable::schedulable_task<void> {
        co_await riz::coro::sleep(0u);
        flag = true;
        co_return;
    }(sched, reached);

    auto t = execution::start(std::move(task));
    sched.run();

    EXPECT_TRUE(reached);
    EXPECT_TRUE(t.done());
}

TEST_F(TimerAwaiterTest, SleepPositiveSuspendsAndResumes) {
    execution::scheduler sched;
    bool reached = false;

    auto task = [](execution::scheduler& sched,
                    bool& flag) -> resumable::schedulable_task<void> {
        co_await riz::coro::sleep(5u);
        flag = true;
        co_return;
    }(sched, reached);

    auto t = execution::start(std::move(task));

    sched.run();
    EXPECT_FALSE(reached);
    EXPECT_FALSE(t.done());

    riz_mock_tick_advance(10);
    ts().run();
    sched.run();

    EXPECT_TRUE(reached);
    EXPECT_TRUE(t.done());
}

TEST_F(TimerAwaiterTest, SleepChronoMillisecondsConvertsCorrectly) {
    execution::scheduler sched;
    bool reached = false;

    auto task = [](execution::scheduler& sched,
                    bool& flag) -> resumable::schedulable_task<void> {
        co_await riz::coro::sleep(std::chrono::milliseconds(5));
        flag = true;
        co_return;
    }(sched, reached);

    auto t = execution::start(std::move(task));

    sched.run();
    EXPECT_FALSE(reached);
    EXPECT_FALSE(t.done());

    riz_mock_tick_advance(10);
    ts().run();
    sched.run();

    EXPECT_TRUE(reached);
    EXPECT_TRUE(t.done());
}

TEST_F(TimerAwaiterTest, SleepChronoSecondsConvertsCorrectly) {
    execution::scheduler sched;
    int value = 0;

    auto task = [](execution::scheduler& sched,
                    int& val) -> resumable::schedulable_task<void> {
        co_await riz::coro::sleep(std::chrono::seconds(1));
        val = 42;
        co_return;
    }(sched, value);

    auto t = execution::start(std::move(task));

    sched.run();
    EXPECT_EQ(value, 0);
    EXPECT_FALSE(t.done());

    riz_mock_tick_advance(1100);
    ts().run();
    sched.run();

    EXPECT_EQ(value, 42);
    EXPECT_TRUE(t.done());
}

TEST_F(TimerAwaiterTest, SleepZeroChronoDoesNotSuspend) {
    execution::scheduler sched;
    bool reached = false;

    auto task = [](execution::scheduler& sched,
                    bool& flag) -> resumable::schedulable_task<void> {
        co_await riz::coro::sleep(std::chrono::milliseconds(0));
        flag = true;
        co_return;
    }(sched, reached);

    auto t = execution::start(std::move(task));
    sched.run();

    EXPECT_TRUE(reached);
    EXPECT_TRUE(t.done());
}

TEST_F(TimerAwaiterTest, MultipleSleepInSequence) {
    execution::scheduler sched;
    std::vector<int> sequence;

    auto task =
        [](execution::scheduler& sched,
            std::vector<int>& seq) -> resumable::schedulable_task<void> {
        seq.push_back(1);
        co_await riz::coro::sleep(5u);
        seq.push_back(2);
        co_await riz::coro::sleep(3u);
        seq.push_back(3);
        co_return;
    }(sched, sequence);

    auto t = execution::start(std::move(task));

    sched.run();
    ASSERT_EQ(sequence.size(), 1u);
    EXPECT_EQ(sequence[0], 1);

    riz_mock_tick_advance(10);
    ts().run();
    sched.run();

    ASSERT_EQ(sequence.size(), 2u);
    EXPECT_EQ(sequence[1], 2);

    riz_mock_tick_advance(10);
    ts().run();
    sched.run();

    ASSERT_EQ(sequence.size(), 3u);
    EXPECT_EQ(sequence[2], 3);
    EXPECT_TRUE(t.done());
}

TEST_F(TimerAwaiterTest, CoroutineValuePreservedAfterSleep) {
    execution::scheduler sched;

    auto task =
        [](execution::scheduler& sched) -> resumable::schedulable_task<int> {
        int x = 10;
        int y = 20;
        co_await riz::coro::sleep(3u);
        co_return x + y;
    }(sched);

    auto t = execution::start(std::move(task));

    sched.run();
    EXPECT_FALSE(t.done());

    riz_mock_tick_advance(10);
    ts().run();
    sched.run();

    EXPECT_TRUE(t.done());
    EXPECT_EQ(t.take_result(), 30);
}

TEST_F(TimerAwaiterTest, SleepAwaitableReadyWhenZeroTicks) {
    awaiter::timer_awaiter a {0u};
    EXPECT_TRUE(a.await_ready());
}

TEST_F(TimerAwaiterTest, SleepAwaitableNotReadyWhenPositiveTicks) {
    awaiter::timer_awaiter a {5u};
    EXPECT_FALSE(a.await_ready());
}

TEST_F(TimerAwaiterTest, SleepAwaiterSizeIsTight) {
    EXPECT_LE(sizeof(awaiter::timer_awaiter), 64u);
}
