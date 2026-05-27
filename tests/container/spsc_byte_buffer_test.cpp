#include <gtest/gtest.h>
#include <riz/container/lockfree/spsc_byte_buffer.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstring>
#include <thread>
#include <vector>

using namespace riz::container::lockfree;

class SpscByteBufferTest : public ::testing::Test {
protected:
    static constexpr std::size_t kCapacity = 16;
    spsc_byte_buffer<kCapacity> buf_;
};

TEST_F(SpscByteBufferTest, InitialState) {
    EXPECT_EQ(buf_.size(), 0);
    EXPECT_EQ(buf_.capacity(), kCapacity);
}

TEST_F(SpscByteBufferTest, PushZeroLength) {
    const std::byte data[] = {std::byte {0x00}};
    EXPECT_TRUE(buf_.push(data, 0));
    EXPECT_EQ(buf_.size(), 0);
}

TEST_F(SpscByteBufferTest, PopFromEmpty) {
    std::byte out[4] = {};
    std::size_t read = buf_.pop_front(out, sizeof(out));
    EXPECT_EQ(read, 0);
    EXPECT_EQ(buf_.size(), 0);
}

TEST_F(SpscByteBufferTest, PopZeroLength) {
    const std::byte data[] = {std::byte {0xAA}};
    ASSERT_TRUE(buf_.push(data, 1));

    std::byte out = std::byte {0x00};
    EXPECT_EQ(buf_.pop_front(&out, 0), 0);
    EXPECT_EQ(buf_.size(), 1);
}

TEST_F(SpscByteBufferTest, PushAndPopSingleByte) {
    const std::byte in = std::byte {0xAB};
    ASSERT_TRUE(buf_.push(&in, 1));
    EXPECT_EQ(buf_.size(), 1);

    std::byte out = std::byte {0x00};
    std::size_t read = buf_.pop_front(&out, 1);
    EXPECT_EQ(read, 1);
    EXPECT_EQ(out, std::byte {0xAB});
    EXPECT_EQ(buf_.size(), 0);
}

TEST_F(SpscByteBufferTest, PushAndPopMultipleBytes) {
    const std::byte data[] = {std::byte {0x01}, std::byte {0x02}, std::byte {0x03}};
    ASSERT_TRUE(buf_.push(data, 3));
    EXPECT_EQ(buf_.size(), 3);

    std::byte out[3] = {};
    std::size_t read = buf_.pop_front(out, sizeof(out));
    EXPECT_EQ(read, 3);
    EXPECT_EQ(memcmp(out, data, 3), 0);
    EXPECT_EQ(buf_.size(), 0);
}

TEST_F(SpscByteBufferTest, PopLessThanAvailable) {
    const std::byte data[] = {
        std::byte {0x01}, std::byte {0x02}, std::byte {0x03}, std::byte {0x04}};
    ASSERT_TRUE(buf_.push(data, 4));

    std::byte out[2] = {};
    std::size_t read = buf_.pop_front(out, 2);
    EXPECT_EQ(read, 2);
    EXPECT_EQ(out[0], std::byte {0x01});
    EXPECT_EQ(out[1], std::byte {0x02});
    EXPECT_EQ(buf_.size(), 2);
}

TEST_F(SpscByteBufferTest, PopMoreThanAvailable) {
    const std::byte data[] = {std::byte {0x01}, std::byte {0x02}};
    ASSERT_TRUE(buf_.push(data, 2));

    std::byte out[8] = {};
    std::size_t read = buf_.pop_front(out, sizeof(out));
    EXPECT_EQ(read, 2);
    EXPECT_EQ(out[0], std::byte {0x01});
    EXPECT_EQ(out[1], std::byte {0x02});
    EXPECT_EQ(buf_.size(), 0);
}

TEST_F(SpscByteBufferTest, PushFailsWhenFull) {
    static constexpr std::size_t kUsable = kCapacity - 1;
    std::vector<std::byte> fill(kUsable, std::byte {0xFF});
    ASSERT_TRUE(buf_.push(fill.data(), fill.size()));
    EXPECT_EQ(buf_.size(), kUsable);

    const std::byte overflow = std::byte {0xEE};
    EXPECT_FALSE(buf_.push(&overflow, 1));
    EXPECT_EQ(buf_.size(), kUsable);
}

TEST_F(SpscByteBufferTest, PushTooLargeFails) {
    std::vector<std::byte> large(kCapacity + 1, std::byte {0xAA});
    EXPECT_FALSE(buf_.push(large.data(), large.size()));
    EXPECT_EQ(buf_.size(), 0);
}

TEST_F(SpscByteBufferTest, WrapAroundWrite) {
    std::vector<std::byte> first(kCapacity - 2, std::byte {0x11});
    ASSERT_TRUE(buf_.push(first.data(), first.size()));

    std::vector<std::byte> discard(kCapacity - 2, std::byte {0x00});
    std::size_t read = buf_.pop_front(discard.data(), discard.size());
    ASSERT_EQ(read, kCapacity - 2);
    EXPECT_EQ(buf_.size(), 0);

    const std::byte wrap[] = {std::byte {0x22}, std::byte {0x33}, std::byte {0x44}};
    ASSERT_TRUE(buf_.push(wrap, sizeof(wrap)));
    EXPECT_EQ(buf_.size(), 3);

    std::byte out[3] = {};
    read = buf_.pop_front(out, sizeof(out));
    EXPECT_EQ(read, 3);
    EXPECT_EQ(memcmp(out, wrap, sizeof(wrap)), 0);
}

TEST_F(SpscByteBufferTest, FullWrapAroundCycle) {
    for (int round = 0; round < 10; ++round) {
        std::byte val = static_cast<std::byte>(round & 0xFF);
        std::vector<std::byte> in(kCapacity - 1, val);
        ASSERT_TRUE(buf_.push(in.data(), in.size()));
        EXPECT_EQ(buf_.size(), kCapacity - 1);

        std::vector<std::byte> out(kCapacity - 1, std::byte {0x00});
        std::size_t read = buf_.pop_front(out.data(), out.size());
        ASSERT_EQ(read, kCapacity - 1);
        for (std::size_t i = 0; i < out.size(); ++i) {
            EXPECT_EQ(out[i], val) << "round=" << round << " i=" << i;
        }
        EXPECT_EQ(buf_.size(), 0);
    }
}

TEST_F(SpscByteBufferTest, SequentialPushPopInterleaved) {
    for (std::size_t i = 0; i < 100; ++i) {
        std::byte b = static_cast<std::byte>(i & 0xFF);
        ASSERT_TRUE(buf_.push(&b, 1));

        std::byte out = std::byte {0x00};
        std::size_t read = buf_.pop_front(&out, 1);
        ASSERT_EQ(read, 1);
        EXPECT_EQ(out, b);
    }
    EXPECT_EQ(buf_.size(), 0);
}

TEST_F(SpscByteBufferTest, SmallCapacityBuffer) {
    spsc_byte_buffer<2> small;
    EXPECT_EQ(small.capacity(), 2);

    const std::byte a = std::byte {0x01};
    EXPECT_TRUE(small.push(&a, 1));
    EXPECT_EQ(small.size(), 1);

    std::byte out = std::byte {0x00};
    EXPECT_EQ(small.pop_front(&out, 1), 1);
    EXPECT_EQ(out, std::byte {0x01});
    EXPECT_EQ(small.size(), 0);
}

TEST_F(SpscByteBufferTest, ConcurrentSpsc) {
    static constexpr std::size_t kBufCap = 4096;
    static constexpr std::size_t kTotalBytes = 1'000'000;
    spsc_byte_buffer<kBufCap> ring;

    std::thread producer([&] {
        std::size_t produced = 0;
        while (produced < kTotalBytes) {
            std::byte val = static_cast<std::byte>(produced & 0xFF);
            if (ring.push(&val, 1)) {
                ++produced;
            }
        }
    });

    std::thread consumer([&] {
        std::size_t consumed = 0;
        while (consumed < kTotalBytes) {
            std::byte out = std::byte {0x00};
            std::size_t read = ring.pop_front(&out, 1);
            if (read > 0) {
                EXPECT_EQ(out, static_cast<std::byte>(consumed & 0xFF))
                    << "consumed=" << consumed;
                ++consumed;
            }
        }
    });

    producer.join();
    consumer.join();
    EXPECT_EQ(ring.size(), 0);
}

TEST_F(SpscByteBufferTest, ConcurrentSpscBatched) {
    static constexpr std::size_t kBufCap = 8192;
    static constexpr std::size_t kBatchSize = 64;
    static constexpr std::size_t kNumBatches = 5000;
    spsc_byte_buffer<kBufCap> ring;

    std::thread producer([&] {
        std::array<std::byte, kBatchSize> batch {};
        for (std::size_t b = 0; b < kNumBatches; ++b) {
            for (std::size_t i = 0; i < kBatchSize; ++i) {
                batch[i] = static_cast<std::byte>((b * kBatchSize + i) & 0xFF);
            }
            while (!ring.push(batch.data(), kBatchSize)) {
            }
        }
    });

    std::thread consumer([&] {
        std::array<std::byte, kBatchSize> batch {};
        for (std::size_t b = 0; b < kNumBatches;) {
            std::size_t read = ring.pop_front(batch.data(), kBatchSize);
            if (read == kBatchSize) {
                for (std::size_t i = 0; i < kBatchSize; ++i) {
                    auto expected = static_cast<std::byte>((b * kBatchSize + i) & 0xFF);
                    EXPECT_EQ(batch[i], expected)
                        << "batch=" << b << " i=" << i;
                }
                ++b;
            }
        }
    });

    producer.join();
    consumer.join();
    EXPECT_EQ(ring.size(), 0);
}
