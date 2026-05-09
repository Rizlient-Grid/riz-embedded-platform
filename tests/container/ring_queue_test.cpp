#include <gtest/gtest.h>
#include <riz/container/ring_queue.hpp>

using namespace riz::container;

class RingQueueTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(RingQueueTest, Initialization) {
    int buffer[5];
    ring_queue<int> q(buffer);

    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0);
    EXPECT_EQ(q.capacity(), 5);
}

TEST_F(RingQueueTest, PushAndPopSingle) {
    int buffer[3];
    ring_queue<int> q(buffer);

    q.push(42);
    EXPECT_FALSE(q.empty());
    EXPECT_EQ(q.size(), 1);

    int val = 0;
    EXPECT_TRUE(q.pop_front(val));
    EXPECT_EQ(val, 42);
    EXPECT_TRUE(q.empty());
    EXPECT_EQ(q.size(), 0);
}

TEST_F(RingQueueTest, PushMultiple) {
    int buffer[3];
    ring_queue<int> q(buffer);

    q.push(1);
    q.push(2);
    q.push(3);

    EXPECT_EQ(q.size(), 3);

    int val;
    EXPECT_TRUE(q.pop_front(val));
    EXPECT_EQ(val, 1);
    EXPECT_TRUE(q.pop_front(val));
    EXPECT_EQ(val, 2);
    EXPECT_TRUE(q.pop_front(val));
    EXPECT_EQ(val, 3);
    EXPECT_TRUE(q.empty());
}

TEST_F(RingQueueTest, OverwriteWhenFull) {
    int buffer[3];
    ring_queue<int> q(buffer);

    q.push(1);
    q.push(2);
    q.push(3);

    // Now it's full. Pushing 4 should overwrite 1.
    q.push(4);
    EXPECT_EQ(q.size(), 3);
    EXPECT_EQ(q.capacity(), 3);

    int val;
    // Expected order: 2, 3, 4
    EXPECT_TRUE(q.pop_front(val));
    EXPECT_EQ(val, 2);
    EXPECT_TRUE(q.pop_front(val));
    EXPECT_EQ(val, 3);

    // Push another while not fully empty
    q.push(5);
    EXPECT_EQ(q.size(), 2);

    EXPECT_TRUE(q.pop_front(val));
    EXPECT_EQ(val, 4);
    EXPECT_TRUE(q.pop_front(val));
    EXPECT_EQ(val, 5);
    EXPECT_TRUE(q.empty());
}

TEST_F(RingQueueTest, PopEmpty) {
    int buffer[2];
    ring_queue<int> q(buffer);

    int val = 100;
    EXPECT_FALSE(q.pop_front(val));
    EXPECT_EQ(val, 100); // Value should remain unchanged
}

TEST_F(RingQueueTest, WrapAround) {
    int buffer[3];
    ring_queue<int> q(buffer);

    // Push 3 elements: head = 0, tail = 0 (wrapped), size = 3
    q.push(1);
    q.push(2);
    q.push(3);

    // Pop 2 elements: head = 2, tail = 0, size = 1
    int val;
    q.pop_front(val);
    q.pop_front(val);

    // Push 2 more: head = 2, tail = 2, size = 3
    q.push(4);
    q.push(5);

    EXPECT_EQ(q.size(), 3);

    // Remaining elements should be 3, 4, 5
    EXPECT_TRUE(q.pop_front(val));
    EXPECT_EQ(val, 3);
    EXPECT_TRUE(q.pop_front(val));
    EXPECT_EQ(val, 4);
    EXPECT_TRUE(q.pop_front(val));
    EXPECT_EQ(val, 5);
    EXPECT_TRUE(q.empty());
}

struct Point {
    int x, y;
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

TEST_F(RingQueueTest, CustomTriviallyCopyableType) {
    Point buffer[2];
    ring_queue<Point> q(buffer);

    q.push({1, 2});
    q.push({3, 4});
    q.push({5, 6}); // Overwrites {1, 2}

    Point p;
    EXPECT_TRUE(q.pop_front(p));
    EXPECT_EQ(p, (Point {3, 4}));
    EXPECT_TRUE(q.pop_front(p));
    EXPECT_EQ(p, (Point {5, 6}));
}
