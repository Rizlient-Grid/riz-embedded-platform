#include <gtest/gtest.h>

#include <riz/timer/timer_node.h>
#include <riz/timer/timer_service.h>

#include "mock_tick_source.h"

#include <cstdint>

using namespace riz;

struct expirable_node : timer::timer_node {
    int* fired;
};

namespace {

void on_expire_callback(timer::timer_node* tn) noexcept {
    auto* en = static_cast<expirable_node*>(tn);
    *en->fired = 1;
}

timer::timer_service& ts() {
    return timer::timer_service::instance();
}

} // namespace

class TimerServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        riz_mock_tick_set(0);
        ts().run();
    }
};

TEST_F(TimerServiceTest, RunOnEmptyQueueIsNoOp) {
    int fired = 0;
    expirable_node n;
    n.fired = &fired;
    n.on_expire = on_expire_callback;

    ts().run();

    EXPECT_EQ(fired, 0);
}

TEST_F(TimerServiceTest, SubmitZeroDelayFiresImmediatelyOnRun) {
    int fired = 0;
    expirable_node n;
    n.fired = &fired;
    n.on_expire = on_expire_callback;

    ts().submit(0, n);
    ts().run();

    EXPECT_EQ(fired, 1);
}

TEST_F(TimerServiceTest, SubmitAndRunSingleTimer) {
    int fired = 0;
    expirable_node n;
    n.fired = &fired;
    n.on_expire = on_expire_callback;

    ts().submit(5, n);
    riz_mock_tick_advance(10);
    ts().run();

    EXPECT_EQ(fired, 1);
}

TEST_F(TimerServiceTest, TimerDoesNotFireBeforeExpiry) {
    int fired = 0;
    expirable_node n;
    n.fired = &fired;
    n.on_expire = on_expire_callback;

    ts().submit(50, n);
    riz_mock_tick_advance(30);
    ts().run();

    EXPECT_EQ(fired, 0);

    ts().cancel(n);
}

TEST_F(TimerServiceTest, CancelPreventsFire) {
    int fired = 0;
    expirable_node n;
    n.fired = &fired;
    n.on_expire = on_expire_callback;

    ts().submit(5, n);
    ts().cancel(n);

    riz_mock_tick_advance(10);
    ts().run();

    EXPECT_EQ(fired, 0);
}

TEST_F(TimerServiceTest, CancelThenResubmitSameNode) {
    int fire_count = 0;
    expirable_node n;

    n.fired = &fire_count;
    n.on_expire = [](timer::timer_node* tn) noexcept {
        auto* en = static_cast<expirable_node*>(tn);
        ++(*en->fired);
    };

    ts().submit(5, n);
    ts().cancel(n);

    ts().submit(10, n);
    riz_mock_tick_advance(12);
    ts().run();

    EXPECT_EQ(fire_count, 1);
}

TEST_F(TimerServiceTest, MultipleTimersFireInOrder) {
    expirable_node n1, n2, n3;

    n1.on_expire = [](timer::timer_node* tn) noexcept {
        auto* en = static_cast<expirable_node*>(tn);
        *en->fired = 1;
    };
    n2.on_expire = [](timer::timer_node* tn) noexcept {
        auto* en = static_cast<expirable_node*>(tn);
        *en->fired = 2;
    };
    n3.on_expire = [](timer::timer_node* tn) noexcept {
        auto* en = static_cast<expirable_node*>(tn);
        *en->fired = 3;
    };

    int result1 = 0, result2 = 0, result3 = 0;
    n1.fired = &result1;
    n2.fired = &result2;
    n3.fired = &result3;

    ts().submit(3, n1);
    ts().submit(8, n2);
    ts().submit(14, n3);

    riz_mock_tick_advance(20);
    ts().run();

    EXPECT_EQ(result1, 1);
    EXPECT_EQ(result2, 2);
    EXPECT_EQ(result3, 3);
}

TEST_F(TimerServiceTest, SubmitDoesNotRequireRunForInsert) {
    int fired = 0;
    expirable_node n;
    n.fired = &fired;
    n.on_expire = on_expire_callback;

    ts().submit(100, n);
    EXPECT_EQ(fired, 0);

    ts().cancel(n);
}

TEST_F(TimerServiceTest, RunAdvancesTimeCumulatively) {
    int first_fired = 0;
    int second_fired = 0;
    expirable_node n1, n2;
    n1.fired = &first_fired;
    n1.on_expire = on_expire_callback;
    n2.fired = &second_fired;
    n2.on_expire = on_expire_callback;

    ts().submit(5, n1);
    ts().submit(20, n2);

    riz_mock_tick_advance(10);
    ts().run();
    EXPECT_EQ(first_fired, 1);
    EXPECT_EQ(second_fired, 0);

    riz_mock_tick_advance(15);
    ts().run();
    EXPECT_EQ(second_fired, 1);
}

TEST_F(TimerServiceTest, ConcurrentTimersShareSameKey) {
    int a_fired = 0;
    int b_fired = 0;

    expirable_node na, nb;
    na.on_expire = [](timer::timer_node* tn) noexcept {
        auto* en = static_cast<expirable_node*>(tn);
        ++(*en->fired);
    };
    nb.on_expire = [](timer::timer_node* tn) noexcept {
        auto* en = static_cast<expirable_node*>(tn);
        ++(*en->fired);
    };
    na.fired = &a_fired;
    nb.fired = &b_fired;

    ts().submit(5, na);
    ts().submit(5, nb);

    riz_mock_tick_advance(10);
    ts().run();

    EXPECT_EQ(a_fired, 1);
    EXPECT_EQ(b_fired, 1);
}

TEST_F(TimerServiceTest, CancelNonExistentNodeIsSafe) {
    int fired = 0;
    expirable_node n;
    n.fired = &fired;
    n.on_expire = on_expire_callback;

    ts().cancel(n);
    ts().run();

    EXPECT_EQ(fired, 0);
}

TEST_F(TimerServiceTest, CancelIsGracefulOnEmptyQueue) {
    expirable_node n;
    n.fired = nullptr;
    n.on_expire = on_expire_callback;

    ts().cancel(n);
    ts().run();
}

TEST_F(TimerServiceTest, RunMultipleTimesWithoutNewTimers) {
    for (int i = 0; i < 5; ++i) {
        riz_mock_tick_advance(1);
        ts().run();
    }
}

TEST_F(TimerServiceTest, SubmitLargeDelay) {
    int fired = 0;
    expirable_node n;
    n.fired = &fired;
    n.on_expire = on_expire_callback;

    ts().submit(10000, n);
    riz_mock_tick_advance(5000);
    ts().run();

    EXPECT_EQ(fired, 0);

    riz_mock_tick_advance(6000);
    ts().run();

    EXPECT_EQ(fired, 1);
}

TEST_F(TimerServiceTest, SubmitMsZeroDelayFiresImmediately) {
    int fired = 0;
    expirable_node n;
    n.fired = &fired;
    n.on_expire = on_expire_callback;

    ts().submit_ms(0, n);
    ts().run();

    EXPECT_EQ(fired, 1);
}

TEST_F(TimerServiceTest, SubmitMsSingleTimerFiresAfterExpiry) {
    int fired = 0;
    expirable_node n;
    n.fired = &fired;
    n.on_expire = on_expire_callback;

    ts().submit_ms(5, n);
    riz_mock_tick_advance(10);
    ts().run();

    EXPECT_EQ(fired, 1);
}

TEST_F(TimerServiceTest, SubmitMsDoesNotFireBeforeExpiry) {
    int fired = 0;
    expirable_node n;
    n.fired = &fired;
    n.on_expire = on_expire_callback;

    ts().submit_ms(50, n);
    riz_mock_tick_advance(30);
    ts().run();

    EXPECT_EQ(fired, 0);

    ts().cancel(n);
}

TEST_F(TimerServiceTest, SubmitMsCancelPreventsFire) {
    int fired = 0;
    expirable_node n;
    n.fired = &fired;
    n.on_expire = on_expire_callback;

    ts().submit_ms(5, n);
    ts().cancel(n);

    riz_mock_tick_advance(10);
    ts().run();

    EXPECT_EQ(fired, 0);
}

TEST_F(TimerServiceTest, SubmitMsConsistentWithSubmit) {
    int ms_fired = 0;
    int tick_fired = 0;
    expirable_node n1, n2;
    n1.fired = &ms_fired;
    n1.on_expire = on_expire_callback;
    n2.fired = &tick_fired;
    n2.on_expire = on_expire_callback;

    ts().submit_ms(10, n1);
    ts().submit(10, n2);

    riz_mock_tick_advance(15);
    ts().run();

    EXPECT_EQ(ms_fired, 1);
    EXPECT_EQ(tick_fired, 1);
}
