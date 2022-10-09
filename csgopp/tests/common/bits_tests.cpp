#include <gtest/gtest.h>

#include "csgopp/common/bits.h"

using namespace csgopp::common::bits;

TEST(BitStream, one)
{
    BitStream stream(std::vector<uint8_t>{1});
}

TEST(BitStream, bit)
{
    BitStream stream(std::vector<uint8_t>{0b1010'1010});

    uint8_t value;
    for (uint8_t i = 0; i < 8; ++i)
    {
        EXPECT_TRUE(stream.read(&value, 1));
        EXPECT_EQ(value, i % 2);
    }

    EXPECT_FALSE(stream.read(&value, 1));
}

TEST(BitStream, bits)
{
    BitStream stream(std::vector<uint8_t>{0b0101'0101, 0b0101'0101});
    uint16_t compare = 0b0101'0101'0101'0101;
    uint16_t value;

    for (uint8_t bits = 0; bits <= 16; ++bits)
    {
        EXPECT_TRUE(stream.read(&value, bits));
        EXPECT_EQ(value, compare & (0xFFFF >> (16 - bits)));
        stream.reset();
    }
}

TEST(BitStream, bits_split)
{
    BitStream stream(std::vector<uint8_t>{0b0101'0101, 0b011'0011});
    uint16_t compare = 0b0011'0011'0101'0101;
    uint16_t value;

    for (uint8_t bits = 0; bits <= 16; ++bits)
    {
        EXPECT_TRUE(stream.read(&value, bits));
        EXPECT_EQ(value, compare & (0xFFFF >> (16 - bits)));
        EXPECT_TRUE(stream.read(&value, 16 - bits));
        EXPECT_EQ(value, compare >> bits);
        stream.reset();
    }
}

TEST(BitStream, bounds)
{
    BitStream stream(std::vector<uint8_t>{0b0101'0101, 0b0101'0101});
    uint32_t value;
    EXPECT_FALSE(stream.read(&value, 17));
    EXPECT_FALSE(stream.read(&value, 32));
    EXPECT_FALSE(stream.skip(17));
    EXPECT_FALSE(stream.skip(32));
}
