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

TEST(BitStream, string)
{
    BitStream stream(std::vector<uint8_t>{'a', 'b', 'c', 0});
    std::string value;
    EXPECT_TRUE(stream.read_string(value));
    EXPECT_EQ(value, "abc");
}

TEST(BitStream, string_unterminated)
{
    BitStream stream(std::vector<uint8_t>{'a', 'b', 'c'});
    std::string value;
    EXPECT_FALSE(stream.read_string(value));
}

// def offset(s, b):
//	for c in s:
//		b = bin(ord(c))[2:].rjust(8, "0") + b
//		print("0b" + b[-8:])
//		b = b[:-8]

TEST(BitStream, string_offset)
{
    BitStream stream(std::vector<uint8_t>{0b11000011, 0b11000100, 0b11000110, 0b00000000, 0b00000000});
    std::uint8_t offset;
    std::string value;
    EXPECT_TRUE(stream.read(&offset, 1));
    EXPECT_EQ(offset, 1);
    EXPECT_TRUE(stream.read_string(value));
    EXPECT_EQ(value, "abc");
}
