#include <gtest/gtest.h>

#include "csgopp/common/bits.h"

using namespace csgopp::common::bits;

using Decoder = BitDecoder<BitView>;

TEST(Decoder, endian)
{
    std::vector<uint8_t> data{0xAA, 0xBB, 0xCC, 0xDD};
    Decoder stream(data);
    uint8_t byte;
    EXPECT_TRUE(stream.read(&byte, 8));
    EXPECT_EQ(0xAA, byte);

    stream.reset();
    uint16_t word;
    EXPECT_TRUE(stream.read(&word, 16));
    EXPECT_EQ(0xBBAA, word);

    stream.reset();
    uint32_t dword;
    EXPECT_TRUE(stream.read(&dword, 32));
    EXPECT_EQ(0xDDCCBBAA, dword);

    stream.reset();
    uint64_t size;
    EXPECT_FALSE(stream.read(&size, 64));
}

TEST(Decoder, bit)
{
    // It needs to be 32 bit lmao
    std::vector<uint8_t> data{0b10101010};
    Decoder stream(data);

    uint8_t value;
    for (uint8_t i = 0; i < 8; ++i)
    {
        EXPECT_TRUE(stream.read(&value, 1));
        EXPECT_EQ(value, i % 2);
    }

    EXPECT_FALSE(stream.read(&value, 1));
}

TEST(Decoder, bits)
{
    std::vector<uint8_t> data{0b0101'0101, 0b0101'0101};
    Decoder stream(data);
    uint16_t compare = 0b0101'0101'0101'0101;
    uint16_t value;

    for (uint8_t bits = 0; bits <= 16; ++bits)
    {
        EXPECT_TRUE(stream.read(&value, bits));
        EXPECT_EQ(value, compare & (0xFFFF >> (16 - bits)));
        stream.reset();
    }
}

TEST(Decoder, bits_split)
{
    std::vector<uint8_t> data{0b0101'0101, 0b011'0011};
    Decoder stream(data);
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

TEST(Decoder, bounds)
{
    std::vector<uint8_t> data{0b0101'0101, 0b0101'0101};
    Decoder stream(data);
    uint32_t value;
    EXPECT_FALSE(stream.read(&value, 17));
    EXPECT_FALSE(stream.read(&value, 32));
    EXPECT_FALSE(stream.skip(17));
    EXPECT_FALSE(stream.skip(32));
}

TEST(Decoder, string)
{
    std::vector<uint8_t> data{'a', 'b', 'c', 0};
    Decoder stream(data);
    std::string value;
    EXPECT_TRUE(stream.read_string(value));
    EXPECT_EQ(value, "abc");
}

TEST(Decoder, string_unterminated)
{
    std::vector<uint8_t> data{'a', 'b', 'c'};
    Decoder stream(data);
    std::string value;
    EXPECT_FALSE(stream.read_string(value));
}

TEST(Decoder, string_offset)
{
    std::vector<uint8_t> data{0b11000011, 0b11000100, 0b11000110, 0b00000000, 0b00000000};
    Decoder stream(data);
    std::uint8_t offset;
    std::string value;
    EXPECT_TRUE(stream.read(&offset, 1));
    EXPECT_EQ(offset, 1);
    EXPECT_TRUE(stream.read_string(value));
    EXPECT_EQ(value, "abc");
}

TEST(Decoder, deserialize_variable_size_simple)
{
    std::string data("\x2a\x00", 2);
    Decoder reader(data);
    int32_t value;
    EXPECT_TRUE(reader.read_variable_unsigned_int(&value));
    EXPECT_EQ(value, 42);
}

TEST(Decoder, deserialize_variable_size_complex)
{
    std::string data("\xaa\x00", 2);
    Decoder reader(data);
    int32_t value;
    EXPECT_TRUE(reader.read_variable_unsigned_int(&value));
    EXPECT_EQ(value, 42);
}

TEST(Decoder, deserialize_variable_size_zero)
{
    std::string data("\x00\x00", 2);
    Decoder reader(data);
    int32_t value;
    EXPECT_TRUE(reader.read_variable_unsigned_int(&value));
    EXPECT_EQ(value, 0);
}

TEST(Decoder, deserialize_variable_size_max)
{
    std::string data("\xFF\xFF\xFF\xFF\xFF\x07", 6);
    Decoder reader(data);
    uint32_t value;
    EXPECT_TRUE(reader.read_variable_unsigned_int(&value));
    EXPECT_EQ(value, std::numeric_limits<uint32_t>::max());
}

TEST(Decoder, deserialize_variable_size_max_bad)
{
    std::string data("\xFF\xFF\xFF\xFF\xFF\xFF", 6);
    Decoder reader(data);
    uint32_t value;
    EXPECT_TRUE(reader.read_variable_unsigned_int(&value));
    EXPECT_EQ(value, std::numeric_limits<uint32_t>::max());
}
