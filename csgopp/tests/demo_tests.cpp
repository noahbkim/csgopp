#include <gtest/gtest.h>

#include <csgopp/demo.h>
#include <sstream>

using namespace csgopp::demo;
using csgopp::common::reader::StreamReader;

TEST(Demo, parse_variable_size_simple)
{
    StreamReader<std::stringstream> reader("\x2a\x80");
    EXPECT_EQ(parse_variable_size<int>(reader), 42);
}
