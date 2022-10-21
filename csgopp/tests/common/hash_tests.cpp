#include <gtest/gtest.h>

#include <csgopp/common/hash.h>

using namespace csgopp::common::hash;

TEST(Lookup, create)
{
    Lookup<size_t>::Builder builder;
    builder.add("hello", 1);
    builder.add("world", 2);
    EXPECT_TRUE(builder.build());
}
