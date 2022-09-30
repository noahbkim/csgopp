#include <gtest/gtest.h>
#include <sstream>

#include <demo.hpp>

using namespace csgopp::demo;

TEST(Demo, parse_little_endian) {
    std::stringstream input("\x55\xA4");
    EXPECT_EQ(parse_little_endian<uint16_t>(input), 42069);
}
