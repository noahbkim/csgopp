#include <gtest/gtest.h>

#include <sstream>
#include <limits>

#include <csgopp/demo.h>

using namespace csgopp::demo;
using csgopp::common::reader::StreamReader;

TEST(Demo, deserialize_variable_size_simple)
{
    StreamReader<std::stringstream> reader(std::string("\x2a\x00", 2));
    auto value = VariableSize<int32_t, int32_t>::deserialize(reader);
    EXPECT_EQ(value.value, 42);
    EXPECT_EQ(value.size, 1);
}

TEST(Demo, deserialize_variable_size_complex)
{
    StreamReader<std::stringstream> reader(std::string("\xaa\x00", 2));
    auto value = VariableSize<int32_t, int32_t>::deserialize(reader);
    EXPECT_EQ(value.value, 42);
    EXPECT_EQ(value.size, 2);
}

TEST(Demo, deserialize_variable_size_zero)
{
    StreamReader<std::stringstream> reader(std::string("\x00", 1));
    auto value = VariableSize<int32_t, int32_t>::deserialize(reader);
    EXPECT_EQ(value.value, 0);
    EXPECT_EQ(value.size, 1);
}

TEST(Demo, deserialize_variable_size_max)
{
    StreamReader<std::stringstream> reader("\xFF\xFF\xFF\xFF\xFF\x07");
    auto value = VariableSize<uint32_t, int32_t>::deserialize(reader);
    EXPECT_EQ(value.value, std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(value.size, 6);
}

TEST(Demo, deserialize_variable_size_max_bad)
{
    StreamReader<std::stringstream> reader("\xFF\xFF\xFF\xFF\xFF\xFF");
    auto value = VariableSize<uint32_t, int32_t>::deserialize(reader);
    EXPECT_EQ(value.value, std::numeric_limits<uint32_t>::max());
    EXPECT_EQ(value.size, 6);
}
