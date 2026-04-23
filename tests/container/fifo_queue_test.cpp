#include <gtest/gtest.h>

#include <riz/container/intrusive/fifo_queue.h>

using namespace riz::container::intrusive;

TEST(FifoQueueTest, InitiallyEmpty) {
    fifo_queue q;
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);
}

TEST(FifoQueueTest, PushSingle) {
    fifo_queue q;
    fifo_queue::node_type n;

    q.push(n);

    EXPECT_FALSE(q.empty());
    EXPECT_EQ(q.size(), 1u);
    EXPECT_EQ(n.next, nullptr);
}

TEST(FifoQueueTest, PushMultiple) {
    fifo_queue q;
    fifo_queue::node_type n1, n2, n3;

    q.push(n1);
    q.push(n2);
    q.push(n3);

    EXPECT_EQ(q.size(), 3u);
    EXPECT_EQ(n1.next, &n2);
    EXPECT_EQ(n2.next, &n3);
    EXPECT_EQ(n3.next, nullptr);
}

TEST(FifoQueueTest, PopFrontFromEmpty) {
    fifo_queue q;
    EXPECT_EQ(q.pop_front(), nullptr);
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);
}

TEST(FifoQueueTest, PopFrontSingle) {
    fifo_queue q;
    fifo_queue::node_type n;
    q.push(n);

    auto* popped = q.pop_front();

    EXPECT_EQ(popped, &n);
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);
}

TEST(FifoQueueTest, PopFrontMaintainsOrder) {
    fifo_queue q;
    fifo_queue::node_type n1, n2, n3;
    q.push(n1);
    q.push(n2);
    q.push(n3);

    EXPECT_EQ(q.pop_front(), &n1);
    EXPECT_EQ(q.pop_front(), &n2);
    EXPECT_EQ(q.pop_front(), &n3);
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0u);
}

TEST(FifoQueueTest, PopFrontAllThenPush) {
    fifo_queue q;
    fifo_queue::node_type n1, n2;
    q.push(n1);
    q.push(n2);

    (void)q.pop_front();
    (void)q.pop_front();
    EXPECT_TRUE(q.empty());

    fifo_queue::node_type n3;
    q.push(n3);
    EXPECT_EQ(q.size(), 1u);
    EXPECT_EQ(q.pop_front(), &n3);
}

TEST(FifoQueueTest, InterleavedPushPop) {
    fifo_queue q;
    fifo_queue::node_type n1, n2, n3;

    q.push(n1);
    EXPECT_EQ(q.pop_front(), &n1);
    EXPECT_TRUE(q.empty());

    q.push(n2);
    q.push(n3);
    EXPECT_EQ(q.size(), 2u);
    EXPECT_EQ(q.pop_front(), &n2);
    EXPECT_EQ(q.pop_front(), &n3);
    EXPECT_TRUE(q.empty());
}

TEST(FifoQueueTest, SizeTracksCorrectly) {
    fifo_queue q;
    fifo_queue::node_type nodes[5];

    for (int i = 0; i < 5; ++i) {
        q.push(nodes[i]);
        EXPECT_EQ(q.size(), static_cast<std::size_t>(i + 1));
    }

    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(q.pop_front(), &nodes[i]);
        EXPECT_EQ(q.size(), static_cast<std::size_t>(4 - i));
    }
}
