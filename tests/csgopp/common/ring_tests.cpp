#include <gtest/gtest.h>

#include "csgopp/common/ring.h"

using namespace csgopp::common::ring;

TEST(Ring, fixed_push_back)
{
    Ring<int, 4> ring;
    EXPECT_EQ(ring.size(), 0);
    ring.push_back_overwrite(0);
    EXPECT_EQ(ring.size(), 1);
    ring.push_back_overwrite(1);
    EXPECT_EQ(ring.size(), 2);
    ring.push_back_overwrite(2);
    EXPECT_EQ(ring.size(), 3);
    ring.push_back_overwrite(3);
    EXPECT_EQ(ring.size(), 4);

    for (size_t i = 0; i < 4; ++i)
    {
        EXPECT_EQ(ring.at(i), i);
    }

    EXPECT_THROW(ring.push_back(4), std::runtime_error);
    ring.push_back_overwrite(4);
    for (size_t i = 0; i < 4; ++i)
    {
        EXPECT_EQ(ring.at(i), i + 1);
    }
}

TEST(Ring, fixed_pop_front)
{
    Ring<int, 4> ring;
    ring.push_back_overwrite(0);
    ring.push_back_overwrite(1);
    EXPECT_EQ(ring.size(), 2);
    ring.pop_front();
    EXPECT_EQ(ring.size(), 1);
    EXPECT_EQ(ring.at(0), 1);
    ring.push_back_overwrite(2);
    ring.push_back_overwrite(3);
    ring.push_back_overwrite(4);
    EXPECT_EQ(ring.size(), 4);
    ring.pop_front();
    ring.pop_front();
    ring.pop_front();
    EXPECT_EQ(ring.size(), 1);
    EXPECT_EQ(ring.at(0), 4);
}

TEST(Ring, runtime_push_back)
{
    Ring<int, Runtime> ring(4);
    EXPECT_EQ(ring.size(), 0);
    ring.push_back_overwrite(0);
    EXPECT_EQ(ring.size(), 1);
    ring.push_back_overwrite(1);
    EXPECT_EQ(ring.size(), 2);
    ring.push_back_overwrite(2);
    EXPECT_EQ(ring.size(), 3);
    ring.push_back_overwrite(3);
    EXPECT_EQ(ring.size(), 4);

    for (size_t i = 0; i < 4; ++i)
    {
        EXPECT_EQ(ring.at(i), i);
    }

    EXPECT_THROW(ring.push_back(4), std::runtime_error);
    ring.push_back_overwrite(4);
    for (size_t i = 0; i < 4; ++i)
    {
        EXPECT_EQ(ring.at(i), i + 1);
    }
}
