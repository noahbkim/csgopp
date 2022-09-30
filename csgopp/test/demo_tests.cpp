#include <gtest/gtest.h>
#include <sstream>

#include <demo.hpp>

using namespace csgopp::demo;

TEST(Demo, parse_little_endian_simple) {
    std::stringstream input("\xa4\x01");
    EXPECT_EQ(parse_little_endian<uint16_t>(input), 420);
}

TEST(Demo, parse_little_endian_negative) {
    std::stringstream input("\xab[\xff\xff");
    EXPECT_EQ(parse_little_endian<int32_t>(input), -42069);
}

TEST(Demo, parse_little_endian_large) {
    std::stringstream input("\x8b(?\x05\xff\xff\xff\xff");
    EXPECT_EQ(parse_little_endian<int64_t>(input), -4206942069LL);
}
