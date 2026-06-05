#include <gtest/gtest.h>

#include <riz/coro/channel/channel.hpp>
#include <riz/coro/execution/execute.hpp>
#include <riz/coro/execution/scheduler.h>
#include <riz/coro/resumable/schedulable_task.hpp>
#include <riz/errcode.h>

#include <cstdint>
#include <type_traits>
#include <vector>

using namespace riz::coro;

using ec = riz::errcode;

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
    int storage[4];
    channel::channel<int> ch(storage);
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
// try_send / try_read: buffered channel (Capacity > 0)
// ---------------------------------------------------------------------------

TEST(ChannelTest, BufferedTrySendTryReceive) {
    int storage[2];
    channel::channel<int> ch(storage);

    EXPECT_EQ(ch.try_send(10), ec::success);
    EXPECT_EQ(ch.try_send(20), ec::success);
    EXPECT_EQ(ch.try_send(30), ec::full);

    int val = 0;
    EXPECT_EQ(ch.try_read(val), ec::success);
    EXPECT_EQ(val, 10);

    EXPECT_EQ(ch.try_read(val), ec::success);
    EXPECT_EQ(val, 20);

    EXPECT_EQ(ch.try_read(val), ec::empty);
}

TEST(ChannelTest, BufferedFIFOOrder) {
    int storage[4];
    channel::channel<int> ch(storage);

    for (int i = 1; i <= 4; ++i) {
        EXPECT_EQ(ch.try_send(i), ec::success);
    }

    int val = 0;
    for (int i = 1; i <= 4; ++i) {
        EXPECT_EQ(ch.try_read(val), ec::success);
        EXPECT_EQ(val, i);
    }
}

TEST(ChannelTest, BufferedRefusesSendAfterClose) {
    int storage[2];
    channel::channel<int> ch(storage);
    ch.close();
    EXPECT_EQ(ch.try_send(1), ec::closed);
}

TEST(ChannelTest, BufferedDrainsPendingOnClose) {
    execution::scheduler sched;
    int storage[1];
    channel::channel<int> ch(storage);

    bool receiver_reached = false;
    auto received_status = static_cast<riz::errcode>(99);

    auto recv_task = [&](execution::scheduler& s) -> resumable::schedulable_task<void> {
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
    EXPECT_EQ(received_status, ec::canceled);
    EXPECT_TRUE(t.done());
}

// ---------------------------------------------------------------------------
// try_send / try_read: unbuffered channel (Capacity == 0, rendezvous)
// ---------------------------------------------------------------------------

TEST(ChannelTest, UnbufferedTrySendFailsNoReceiver) {
    channel::channel<int> ch;
    EXPECT_EQ(ch.try_send(1), ec::full);
}

TEST(ChannelTest, UnbufferedTryReceiveFailsNoSender) {
    channel::channel<int> ch;
    int val = 0;
    EXPECT_EQ(ch.try_read(val), ec::empty);
}

TEST(ChannelTest, UnbufferedRefusesSendAfterClose) {
    channel::channel<int> ch;
    ch.close();
    EXPECT_EQ(ch.try_send(1), ec::closed);
}

TEST(ChannelTest, UnbufferedRefusesReceiveAfterClose) {
    channel::channel<int> ch;
    int val = 0;
    ch.close();
    EXPECT_EQ(ch.try_read(val), ec::closed);
}

// ---------------------------------------------------------------------------
// co_await send: rendezvous with a pending receiver
// ---------------------------------------------------------------------------

namespace {

resumable::schedulable_task<void> sender_task(
    execution::scheduler& sched, channel::channel<int>& ch, int value) {
    riz::errcode rc = co_await ch.send(value);
    EXPECT_EQ(rc, ec::success);
    co_return;
}

resumable::schedulable_task<void> receiver_task(
    execution::scheduler& sched, channel::channel<int>& ch, int& out) {
    riz::errcode rc = co_await ch.receive(out);
    EXPECT_EQ(rc, ec::success);
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
    int storage[2];
    channel::channel<int> ch(storage);

    auto task = [&](execution::scheduler& s) -> resumable::schedulable_task<void> {
        riz::errcode rc = co_await ch.send(7);
        EXPECT_EQ(rc, ec::success);
        co_return;
    };

    auto t = execution::start(task(sched));
    sched.run();

    EXPECT_TRUE(t.done());

    int val = 0;
    EXPECT_EQ(ch.try_read(val), ec::success);
    EXPECT_EQ(val, 7);
}

// ---------------------------------------------------------------------------
// Multiple senders / receivers
// ---------------------------------------------------------------------------

TEST(ChannelTest, MultipleRendezvousPairs) {
    execution::scheduler sched;
    channel::channel<int> ch;
    std::vector<int> results;

    auto send_fn = [&](execution::scheduler&, int v) -> resumable::schedulable_task<void> {
        co_await ch.send(v);
        co_return;
    };

    auto recv_fn = [&](execution::scheduler&, int& out) -> resumable::schedulable_task<void> {
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
    riz::errcode status {};

    auto task = [&](execution::scheduler&) -> resumable::schedulable_task<void> {
        status = co_await ch.send(1);
        co_return;
    };

    auto t = execution::start(task(sched));
    sched.run();
    EXPECT_FALSE(t.done());
    EXPECT_EQ(status, ec::success);

    ch.close();
    sched.run();

    EXPECT_TRUE(t.done());
    EXPECT_EQ(status, ec::canceled);
}

// ---------------------------------------------------------------------------
// close() wakes pending receivers with status -1
// ---------------------------------------------------------------------------

TEST(ChannelTest, CloseWakesPendingReceivers) {
    execution::scheduler sched;
    channel::channel<int> ch;
    riz::errcode status {};

    auto task = [&](execution::scheduler&) -> resumable::schedulable_task<void> {
        int val = 0;
        status = co_await ch.receive(val);
        co_return;
    };

    auto t = execution::start(task(sched));
    sched.run();
    EXPECT_FALSE(t.done());
    EXPECT_EQ(status, ec::success);

    ch.close();
    sched.run();

    EXPECT_TRUE(t.done());
    EXPECT_EQ(status, ec::canceled);
}

// ---------------------------------------------------------------------------
// send() on already-closed channel returns -1 immediately
// ---------------------------------------------------------------------------

TEST(ChannelTest, SendOnClosedChannelReturnsMinusOne) {
    execution::scheduler sched;
    channel::channel<int> ch;
    ch.close();

    auto task = [&](execution::scheduler&) -> resumable::schedulable_task<void> {
        riz::errcode rc = co_await ch.send(1);
        EXPECT_EQ(rc, ec::closed);
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

    auto task = [&](execution::scheduler&) -> resumable::schedulable_task<void> {
        int val = 0;
        riz::errcode rc = co_await ch.receive(val);
        EXPECT_EQ(rc, ec::closed);
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
    int storage[1];
    channel::channel<int> ch(storage);

    auto sender_status = static_cast<riz::errcode>(99);

    auto send_fn = [&](execution::scheduler&) -> resumable::schedulable_task<void> {
        sender_status = co_await ch.send(10);
        co_return;
    };

    auto s1 = execution::start(send_fn(sched));
    sched.run();
    EXPECT_TRUE(s1.done());
    EXPECT_EQ(sender_status, ec::success);

    auto s2 = execution::start(send_fn(sched));
    sched.run();
    EXPECT_FALSE(s2.done());

    int val = 0;
    auto recv_fn = [&](execution::scheduler&, int& out) -> resumable::schedulable_task<void> {
        riz::errcode rc = co_await ch.receive(out);
        EXPECT_EQ(rc, ec::success);
        co_return;
    };

    auto r = execution::start(recv_fn(sched, val));
    sched.run();

    EXPECT_TRUE(s2.done());
    EXPECT_TRUE(r.done());
    EXPECT_EQ(val, 10);
    EXPECT_EQ(sender_status, ec::success);
}

// ---------------------------------------------------------------------------
// Different trivially copyable types
// ---------------------------------------------------------------------------

TEST(ChannelTest, FloatType) {
    float storage[2];
    channel::channel<float> ch(storage);
    EXPECT_EQ(ch.try_send(3.14f), ec::success);

    float val = 0;
    EXPECT_EQ(ch.try_read(val), ec::success);
    EXPECT_FLOAT_EQ(val, 3.14f);
}

TEST(ChannelTest, Uint8Type) {
    uint8_t storage[4];
    channel::channel<uint8_t> ch(storage);
    EXPECT_EQ(ch.try_send(42), ec::success);

    uint8_t val = 0;
    EXPECT_EQ(ch.try_read(val), ec::success);
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
    point storage[2];
    channel::channel<point> ch(storage);
    EXPECT_EQ(ch.try_send(point {3, 4}), ec::success);

    point val {};
    EXPECT_EQ(ch.try_read(val), ec::success);
    EXPECT_EQ(val.x, 3);
    EXPECT_EQ(val.y, 4);
}
