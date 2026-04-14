#include <gtest/gtest.h>

#include <riz/container/intrusive/delta_queue.h>

#include <cstdint>
#include <vector>

using namespace riz::container::intrusive;

TEST(DeltaQueueTest, InitiallyEmpty)
{
    delta_queue dq;
    EXPECT_TRUE(dq.empty());
    EXPECT_EQ(dq.size(), 0u);
}

TEST(DeltaQueueTest, InsertSingleNode)
{
    delta_queue dq;
    delta_queue::node n;

    dq.insert(5, n);

    EXPECT_FALSE(dq.empty());
    EXPECT_EQ(dq.size(), 1u);
    EXPECT_EQ(n.delta, 5u);
    EXPECT_EQ(n.next, nullptr);
}

TEST(DeltaQueueTest, InsertAtHead)
{
    delta_queue dq;
    delta_queue::node n1, n2;

    dq.insert(10, n1);
    dq.insert(3, n2);

    EXPECT_EQ(dq.size(), 2u);
    EXPECT_EQ(n2.delta, 3u);
    EXPECT_EQ(n1.delta, 7u);
    EXPECT_EQ(n2.next, &n1);
    EXPECT_EQ(n1.next, nullptr);
}

TEST(DeltaQueueTest, InsertAtTail)
{
    delta_queue dq;
    delta_queue::node n1, n2, n3;

    dq.insert(3, n1);
    dq.insert(8, n2);
    dq.insert(12, n3);

    EXPECT_EQ(dq.size(), 3u);
    EXPECT_EQ(n1.delta, 3u);
    EXPECT_EQ(n2.delta, 5u);
    EXPECT_EQ(n3.delta, 4u);
    EXPECT_EQ(n1.next, &n2);
    EXPECT_EQ(n2.next, &n3);
    EXPECT_EQ(n3.next, nullptr);
}

TEST(DeltaQueueTest, InsertAtMiddle)
{
    delta_queue dq;
    delta_queue::node n1, n2, n3;

    dq.insert(5, n1);
    dq.insert(12, n3);
    dq.insert(8, n2);

    EXPECT_EQ(dq.size(), 3u);
    EXPECT_EQ(n1.delta, 5u);
    EXPECT_EQ(n2.delta, 3u);
    EXPECT_EQ(n3.delta, 4u);
    EXPECT_EQ(n1.next, &n2);
    EXPECT_EQ(n2.next, &n3);
    EXPECT_EQ(n3.next, nullptr);
}

TEST(DeltaQueueTest, InsertDuplicateAbsKey)
{
    delta_queue dq;
    delta_queue::node n1, n2;

    dq.insert(5, n1);
    dq.insert(5, n2);

    EXPECT_EQ(dq.size(), 2u);
    EXPECT_EQ(n1.delta, 5u);
    EXPECT_EQ(n2.delta, 0u);
    EXPECT_EQ(n1.next, &n2);
    EXPECT_EQ(n2.next, nullptr);
}

TEST(DeltaQueueTest, PopFrontFromEmpty)
{
    delta_queue dq;
    EXPECT_EQ(dq.pop_front(), nullptr);
    EXPECT_TRUE(dq.empty());
    EXPECT_EQ(dq.size(), 0u);
}

TEST(DeltaQueueTest, PopFrontSingle)
{
    delta_queue dq;
    delta_queue::node n;
    dq.insert(10, n);

    auto* popped = dq.pop_front();

    EXPECT_EQ(popped, &n);
    EXPECT_TRUE(dq.empty());
    EXPECT_EQ(dq.size(), 0u);
}

TEST(DeltaQueueTest, PopFrontMaintainsSize)
{
    delta_queue dq;
    delta_queue::node n1, n2, n3;
    dq.insert(3, n1);
    dq.insert(8, n2);
    dq.insert(12, n3);

    dq.pop_front();

    EXPECT_EQ(dq.size(), 2u);
    EXPECT_EQ(n2.delta, 5u);
}

TEST(DeltaQueueTest, PopFrontAll)
{
    delta_queue dq;
    delta_queue::node n1, n2;
    dq.insert(5, n1);
    dq.insert(10, n2);

    dq.pop_front();
    dq.pop_front();

    EXPECT_TRUE(dq.empty());
    EXPECT_EQ(dq.size(), 0u);
}

TEST(DeltaQueueTest, AdvanceNoExpiry)
{
    delta_queue dq;
    delta_queue::node n1, n2;
    dq.insert(5, n1);
    dq.insert(10, n2);

    int call_count = 0;
    dq.advance(3, [&](delta_queue::node*) { ++call_count; });

    EXPECT_EQ(call_count, 0);
    EXPECT_EQ(dq.size(), 2u);
    EXPECT_EQ(n1.delta, 2u);
    EXPECT_EQ(n2.delta, 5u);
}

TEST(DeltaQueueTest, AdvanceExactExpiry)
{
    delta_queue dq;
    delta_queue::node n1, n2;
    dq.insert(5, n1);
    dq.insert(10, n2);

    std::vector<delta_queue::node*> expired;
    dq.advance(5, [&](delta_queue::node* n) { expired.push_back(n); });

    EXPECT_EQ(expired.size(), 1u);
    EXPECT_EQ(expired[0], &n1);
    EXPECT_EQ(dq.size(), 1u);
    EXPECT_EQ(n2.delta, 5u);
}

TEST(DeltaQueueTest, AdvanceMultipleExpiry)
{
    delta_queue dq;
    delta_queue::node n1, n2, n3;
    dq.insert(3, n1);
    dq.insert(8, n2);
    dq.insert(12, n3);

    std::vector<delta_queue::node*> expired;
    dq.advance(9, [&](delta_queue::node* n) { expired.push_back(n); });

    EXPECT_EQ(expired.size(), 2u);
    EXPECT_EQ(expired[0], &n1);
    EXPECT_EQ(expired[1], &n2);
    EXPECT_EQ(dq.size(), 1u);
    EXPECT_EQ(n3.delta, 3u);
}

TEST(DeltaQueueTest, AdvanceAllExpiry)
{
    delta_queue dq;
    delta_queue::node n1, n2;
    dq.insert(5, n1);
    dq.insert(10, n2);

    std::vector<delta_queue::node*> expired;
    dq.advance(10, [&](delta_queue::node* n) { expired.push_back(n); });

    EXPECT_EQ(expired.size(), 2u);
    EXPECT_TRUE(dq.empty());
}

TEST(DeltaQueueTest, AdvanceOnEmpty)
{
    delta_queue dq;
    int call_count = 0;
    dq.advance(100, [&](delta_queue::node*) { ++call_count; });
    EXPECT_EQ(call_count, 0);
}

TEST(DeltaQueueTest, AdvanceWithZeroElapsed)
{
    delta_queue dq;
    delta_queue::node n;
    dq.insert(5, n);

    int call_count = 0;
    dq.advance(0, [&](delta_queue::node*) { ++call_count; });

    EXPECT_EQ(call_count, 0);
    EXPECT_EQ(dq.size(), 1u);
    EXPECT_EQ(n.delta, 5u);
}

TEST(DeltaQueueTest, InsertAndAdvanceSequence)
{
    delta_queue dq;
    delta_queue::node n1, n2, n3;

    dq.insert(10, n1);
    dq.insert(5, n2);
    dq.insert(15, n3);

    EXPECT_EQ(n2.delta, 5u);
    EXPECT_EQ(n1.delta, 5u);
    EXPECT_EQ(n3.delta, 5u);

    std::vector<delta_queue::node*> expired;
    dq.advance(6, [&](delta_queue::node* n) { expired.push_back(n); });

    ASSERT_EQ(expired.size(), 1u);
    EXPECT_EQ(expired[0], &n2);
    EXPECT_EQ(dq.size(), 2u);
    EXPECT_EQ(n1.delta, 4u);
}
