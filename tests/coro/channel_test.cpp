#include <gtest/gtest.h>

#include <riz/coro/channel/channel.hpp>
#include <riz/coro/execution/execute.hpp>
#include <riz/coro/execution/scheduler.h>
#include <riz/coro/resumable/schedulable_task.hpp>

#include <cstdint>
#include <type_traits>
#include <vector>

using namespace riz::coro;

// ---------------------------------------------------------------------------
// Static type trait tests
// ---------------------------------------------------------------------------

TEST(ChannelTest, IsImmovable) {
    using ch = channel::channel<int>;
    static_assert(!std::is_copy_constructible_v<ch>);
    static_assert(!std::is_move_constructible_v<ch>);
}

TEST(ChannelTest, DefaultCapacityIsZero) {
    channel::channel<int> ch;
    EXPECT_EQ(ch.capacity(), 0u);
}

TEST(ChannelTest, BufferedCapacityMatchesTemplateParameter) {
    channel::channel<int, 4> ch;
    EXPECT_EQ(ch.capacity(), 4u);
}

TEST(ChannelTest, ClosedIsFalseInitially) {
    channel::channel<int> ch;
    EXPECT_FALSE(ch.closed());
}

TEST(ChannelTest, RequiresTriviallyCopyableT) {
    static_assert(std::is_trivially_copyable_v<channel::channel<int>>);
    static_assert(std::is_trivially_copyable_v<channel::channel<float>>);
}

// ---------------------------------------------------------------------------
// try_send / try_receive: buffered channel (Capacity > 0)
// ---------------------------------------------------------------------------

TEST(ChannelTest, BufferedTrySendTryReceive) {
    channel::channel<int, 2> ch;

    EXPECT_TRUE(ch.try_send(10));
    EXPECT_TRUE(ch.try_send(20));
    EXPECT_FALSE(ch.try_send(30));

    int val = 0;
    EXPECT_TRUE(ch.try_receive(val));
    EXPECT_EQ(val, 10);

    EXPECT_TRUE(ch.try_receive(val));
    EXPECT_EQ(val, 20);

    EXPECT_FALSE(ch.try_receive(val));
}

TEST(ChannelTest, BufferedFIFOOrder) {
    channel::channel<int, 4> ch;

    for (int i = 1; i <= 4; ++i) {
        EXPECT_TRUE(ch.try_send(i));
    }

    int val = 0;
    for (int i = 1; i <= 4; ++i) {
        EXPECT_TRUE(ch.try_receive(val));
        EXPECT_EQ(val, i);
    }
}

TEST(ChannelTest, BufferedRefusesSendAfterClose) {
    channel::channel<int, 2> ch;
    ch.close();
    EXPECT_FALSE(ch.try_send(1));
}

TEST(ChannelTest, BufferedDrainsPendingOnClose) {
    execution::scheduler sched;
    channel::channel<int, 1> ch;

    bool receiver_reached = false;
    int received_status = 99;

    auto recv_task = [&](execution::scheduler& s)
        -> resumable::schedulable_task<void> {
        int val = 0;
        received_status = co_await ch.receive(val);
        receiver_reached = true;
        co_return;
    };

    auto t = execution::start(recv_task(sched));
    sched.run();

    EXPECT_FALSE(receiver_reached);
    ch.close();
    sched.run();

    EXPECT_TRUE(receiver_reached);
    EXPECT_EQ(received_status, -1);
    EXPECT_TRUE(t.done());
}

// ---------------------------------------------------------------------------
// try_send / try_receive: unbuffered channel (Capacity == 0, rendezvous)
// ---------------------------------------------------------------------------

TEST(ChannelTest, UnbufferedTrySendFailsNoReceiver) {
    channel::channel<int> ch;
    EXPECT_FALSE(ch.try_send(1));
}

TEST(ChannelTest, UnbufferedTryReceiveFailsNoSender) {
    channel::channel<int> ch;
    int val = 0;
    EXPECT_FALSE(ch.try_receive(val));
}

TEST(ChannelTest, UnbufferedRefusesSendAfterClose) {
    channel::channel<int> ch;
    ch.close();
    EXPECT_FALSE(ch.try_send(1));
}

TEST(ChannelTest, UnbufferedRefusesReceiveAfterClose) {
    channel::channel<int> ch;
    int val = 0;
    ch.close();
    EXPECT_FALSE(ch.try_receive(val));
}

// ---------------------------------------------------------------------------
// co_await send: rendezvous with a pending receiver
// ---------------------------------------------------------------------------

namespace {

resumable::schedulable_task<void> sender_task(
    execution::scheduler& sched, channel::channel<int>& ch, int value) {
    int rc = co_await ch.send(value);
    EXPECT_EQ(rc, 0);
    co_return;
}

resumable::schedulable_task<void> receiver_task(
    execution::scheduler& sched, channel::channel<int>& ch, int& out) {
    int rc = co_await ch.receive(out);
    EXPECT_EQ(rc, 0);
    co_return;
}

} // namespace

TEST(ChannelTest, Rendezvous_ReceiverFirst) {
    execution::scheduler sched;
    channel::channel<int> ch;
    int received = 0;

    auto rt = execution::start(receiver_task(sched, ch, received));
    sched.run();
    EXPECT_FALSE(rt.done());

    auto st = execution::start(sender_task(sched, ch, 42));
    sched.run();

    EXPECT_TRUE(rt.done());
    EXPECT_TRUE(st.done());
    EXPECT_EQ(received, 42);
}

TEST(ChannelTest, Rendezvous_SenderFirst) {
    execution::scheduler sched;
    channel::channel<int> ch;
    int received = 0;

    auto st = execution::start(sender_task(sched, ch, 99));
    sched.run();
    EXPECT_FALSE(st.done());

    auto rt = execution::start(receiver_task(sched, ch, received));
    sched.run();

    EXPECT_TRUE(st.done());
    EXPECT_TRUE(rt.done());
    EXPECT_EQ(received, 99);
}

// ---------------------------------------------------------------------------
// co_await with buffered channel: send succeeds immediately when buffer has
// space
// ---------------------------------------------------------------------------

TEST(ChannelTest, BufferedSendSucceedsImmediately) {
    execution::scheduler sched;
    channel::channel<int, 2> ch;

    auto task = [&](execution::scheduler& s)
        -> resumable::schedulable_task<void> {
        int rc = co_await ch.send(7);
        EXPECT_EQ(rc, 0);
        co_return;
    };

    auto t = execution::start(task(sched));
    sched.run();

    EXPECT_TRUE(t.done());

    int val = 0;
    EXPECT_TRUE(ch.try_receive(val));
    EXPECT_EQ(val, 7);
}

// ---------------------------------------------------------------------------
// Multiple senders / receivers
// ---------------------------------------------------------------------------

TEST(ChannelTest, MultipleRendezvousPairs) {
    execution::scheduler sched;
    channel::channel<int> ch;
    std::vector<int> results;

    auto send_fn = [&](execution::scheduler&, int v)
        -> resumable::schedulable_task<void> {
        co_await ch.send(v);
        co_return;
    };

    auto recv_fn = [&](execution::scheduler&, int& out)
        -> resumable::schedulable_task<void> {
        co_await ch.receive(out);
        co_return;
    };

    int r1 = 0, r2 = 0, r3 = 0;
    auto s1 = execution::start(send_fn(sched, 10));
    auto s2 = execution::start(send_fn(sched, 20));
    auto s3 = execution::start(send_fn(sched, 30));
    sched.run();

    auto rv1 = execution::start(recv_fn(sched, r1));
    sched.run();
    auto rv2 = execution::start(recv_fn(sched, r2));
    sched.run();
    auto rv3 = execution::start(recv_fn(sched, r3));
    sched.run();

    EXPECT_TRUE(s1.done());
    EXPECT_TRUE(s2.done());
    EXPECT_TRUE(s3.done());
    EXPECT_TRUE(rv1.done());
    EXPECT_TRUE(rv2.done());
    EXPECT_TRUE(rv3.done());

    EXPECT_EQ(r1, 10);
    EXPECT_EQ(r2, 20);
    EXPECT_EQ(r3, 30);
}

// ---------------------------------------------------------------------------
// close() wakes pending senders with status -1
// ---------------------------------------------------------------------------

TEST(ChannelTest, CloseWakesPendingSenders) {
    execution::scheduler sched;
    channel::channel<int> ch;
    int status = 0;

    auto task = [&](execution::scheduler&)
        -> resumable::schedulable_task<void> {
        status = co_await ch.send(1);
        co_return;
    };

    auto t = execution::start(task(sched));
    sched.run();
    EXPECT_FALSE(t.done());
    EXPECT_EQ(status, 0);

    ch.close();
    sched.run();

    EXPECT_TRUE(t.done());
    EXPECT_EQ(status, -1);
}

// ---------------------------------------------------------------------------
// close() wakes pending receivers with status -1
// ---------------------------------------------------------------------------

TEST(ChannelTest, CloseWakesPendingReceivers) {
    execution::scheduler sched;
    channel::channel<int> ch;
    int status = 0;

    auto task = [&](execution::scheduler&)
        -> resumable::schedulable_task<void> {
        int val = 0;
        status = co_await ch.receive(val);
        co_return;
    };

    auto t = execution::start(task(sched));
    sched.run();
    EXPECT_FALSE(t.done());
    EXPECT_EQ(status, 0);

    ch.close();
    sched.run();

    EXPECT_TRUE(t.done());
    EXPECT_EQ(status, -1);
}

// ---------------------------------------------------------------------------
// send() on already-closed channel returns -1 immediately
// ---------------------------------------------------------------------------

TEST(ChannelTest, SendOnClosedChannelReturnsMinusOne) {
    execution::scheduler sched;
    channel::channel<int> ch;
    ch.close();

    auto task = [&](execution::scheduler&)
        -> resumable::schedulable_task<void> {
        int rc = co_await ch.send(1);
        EXPECT_EQ(rc, -1);
        co_return;
    };

    auto t = execution::start(task(sched));
    sched.run();

    EXPECT_TRUE(t.done());
}

// ---------------------------------------------------------------------------
// receive() on already-closed channel returns -1 immediately
// ---------------------------------------------------------------------------

TEST(ChannelTest, ReceiveOnClosedChannelReturnsMinusOne) {
    execution::scheduler sched;
    channel::channel<int> ch;
    ch.close();

    auto task = [&](execution::scheduler&)
        -> resumable::schedulable_task<void> {
        int val = 0;
        int rc = co_await ch.receive(val);
        EXPECT_EQ(rc, -1);
        co_return;
    };

    auto t = execution::start(task(sched));
    sched.run();

    EXPECT_TRUE(t.done());
}

// ---------------------------------------------------------------------------
// Buffered channel: pending sender gets flushed when receiver drains buffer
// ---------------------------------------------------------------------------

TEST(ChannelTest, BufferedSenderFlushedOnReceive) {
    execution::scheduler sched;
    channel::channel<int, 1> ch;

    int sender_status = 99;

    auto send_fn = [&](execution::scheduler&)
        -> resumable::schedulable_task<void> {
        sender_status = co_await ch.send(10);
        co_return;
    };

    auto s1 = execution::start(send_fn(sched));
    sched.run();
    EXPECT_TRUE(s1.done());
    EXPECT_EQ(sender_status, 0);

    auto s2 = execution::start(send_fn(sched));
    sched.run();
    EXPECT_FALSE(s2.done());

    int val = 0;
    auto recv_fn = [&](execution::scheduler&, int& out)
        -> resumable::schedulable_task<void> {
        int rc = co_await ch.receive(out);
        EXPECT_EQ(rc, 0);
        co_return;
    };

    auto r = execution::start(recv_fn(sched, val));
    sched.run();

    EXPECT_TRUE(s2.done());
    EXPECT_TRUE(r.done());
    EXPECT_EQ(val, 10);
    EXPECT_EQ(sender_status, 0);
}

// ---------------------------------------------------------------------------
// Different trivially copyable types
// ---------------------------------------------------------------------------

TEST(ChannelTest, FloatType) {
    channel::channel<float, 2> ch;
    EXPECT_TRUE(ch.try_send(3.14f));

    float val = 0;
    EXPECT_TRUE(ch.try_receive(val));
    EXPECT_FLOAT_EQ(val, 3.14f);
}

TEST(ChannelTest, Uint8Type) {
    channel::channel<uint8_t, 4> ch;
    EXPECT_TRUE(ch.try_send(42));

    uint8_t val = 0;
    EXPECT_TRUE(ch.try_receive(val));
    EXPECT_EQ(val, 42);
}

// ---------------------------------------------------------------------------
// Struct type (trivially copyable)
// ---------------------------------------------------------------------------

struct point {
    int x;
    int y;
};

static_assert(std::is_trivially_copyable_v<point>);

TEST(ChannelTest, StructType) {
    channel::channel<point, 2> ch;
    EXPECT_TRUE(ch.try_send(point{3, 4}));

    point val {};
    EXPECT_TRUE(ch.try_receive(val));
    EXPECT_EQ(val.x, 3);
    EXPECT_EQ(val.y, 4);
}
