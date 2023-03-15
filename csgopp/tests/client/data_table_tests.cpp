#include <gtest/gtest.h>

#include <csgopp/client/data_table.h>

using namespace csgopp::client::data_table;


TEST(DataTable, is_array_index_0)
{
    EXPECT_TRUE(is_array_index("000", 0));
    EXPECT_FALSE(is_array_index("0", 0));
    EXPECT_FALSE(is_array_index("00", 0));
    EXPECT_FALSE(is_array_index("", 0));
}

TEST(DataTable, is_array_index_1)
{
    EXPECT_TRUE(is_array_index("001", 1));
    EXPECT_FALSE(is_array_index("1", 1));
    EXPECT_FALSE(is_array_index("01", 1));
    EXPECT_FALSE(is_array_index("0001", 1));
    EXPECT_FALSE(is_array_index("", 1));
    EXPECT_FALSE(is_array_index("a", 1));
    EXPECT_FALSE(is_array_index("1a", 1));
    EXPECT_FALSE(is_array_index("a1a", 1));
    EXPECT_FALSE(is_array_index("aa1", 1));
}

TEST(DataTable, is_array_index_12)
{
    EXPECT_TRUE(is_array_index("012", 12));
    EXPECT_FALSE(is_array_index("12", 12));
    EXPECT_FALSE(is_array_index("0012", 12));
    EXPECT_FALSE(is_array_index("", 12));
}

TEST(DataTable, is_array_index_123)
{
    EXPECT_TRUE(is_array_index("123", 123));
    EXPECT_FALSE(is_array_index("0123", 123));
    EXPECT_FALSE(is_array_index("", 123));
}

TEST(DataTable, is_array_index_1234)
{
    EXPECT_TRUE(is_array_index("1234", 1234));
    EXPECT_FALSE(is_array_index("01234", 1234));
    EXPECT_FALSE(is_array_index("", 1234));
    EXPECT_FALSE(is_array_index("a1234", 1234));
}
