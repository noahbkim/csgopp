#include <gtest/gtest.h>

#include <csgopp/common/reader.h>
#include <string>
#include <sstream>

using namespace csgopp::common::reader;
using namespace std::string_literals;

TEST(Reader, stream_reader_stringstream)
{
    StreamReader<std::stringstream> reader("hello");
    char buffer[5];
    reader.read(buffer, 5);
    EXPECT_EQ(strncmp(buffer, "hello", 5), 0);
}

TEST(Reader, stream_reader_eof)
{
    StreamReader<std::stringstream> reader("hello");
    char buffer[6];
    EXPECT_THROW(reader.read(buffer, 6), ReadEOF);
}

TEST(Reader, stream_reader_integer)
{
    int value = -42069;
    std::string bytes(reinterpret_cast<char*>(&value), sizeof(value));
    StreamReader<std::stringstream> reader(bytes);
    EXPECT_EQ(reader.read<int>(), value);
}

TEST(Reader, stream_reader_size_t)
{
    size_t value = 42069;
    std::string bytes(reinterpret_cast<char*>(&value), sizeof(value));
    StreamReader<std::stringstream> reader(bytes);
    EXPECT_EQ(reader.read<size_t>(), value);
}

TEST(Reader, stream_reader_int_little_endian)
{
    StreamReader<std::stringstream> reader("\xab[\xff\xff");
    EXPECT_EQ((reader.read<int32_t, LittleEndian>()), -42069);
}

TEST(Reader, stream_reader_int64_little_endian)
{
    StreamReader<std::stringstream> reader_1("\x8b(?\x05\xff\xff\xff\xff"s);
    EXPECT_EQ((reader_1.read<int64_t, LittleEndian>()), -4206942069LL);
    StreamReader<std::stringstream> reader_2("u\xd7\xc0\xfa\x00\x00\x00\x00"s);
    EXPECT_EQ((reader_2.read<uint64_t, LittleEndian>()), 4206942069LL);
}

TEST(Reader, stream_reader_int64_big_endian)
{
    StreamReader<std::stringstream> reader_1("\xff\xff\xff\xff\x05?(\x8b"s);
    EXPECT_EQ((reader_1.read<int64_t, BigEndian>()), -4206942069LL);
    StreamReader<std::stringstream> reader_2("\x00\x00\x00\x00\xfa\xc0\xd7u"s);
    EXPECT_EQ((reader_2.read<uint64_t, BigEndian>()), 4206942069LL);
}
