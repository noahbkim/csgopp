#include <gtest/gtest.h>

#include <csgopp/common/id.h>
#include <unordered_map>

using namespace csgopp::common::id;

TEST(Id, two_ids)
{
    class Anonymous {};
    using Id = IdBase<Anonymous>;

    EXPECT_NE(Id(), Id());
}

TEST(Id, two_ids_separate)
{
    struct A
    {
        using Id = IdBase<A>;
        Id id;
    };

    struct B
    {
        using Id = IdBase<B>;
        Id id;
    };

    EXPECT_EQ(static_cast<A::Id::value_type>(A::Id()), static_cast<B::Id::value_type>(B::Id()));
}

TEST(Id, unordered_map)
{
    struct S
    {
        using Id = IdBase<S>;
        Id id;
    };

    S x, y;
    EXPECT_NE(x.id, y.id);
    std::unordered_map<S::Id, S> map;
    map[x.id] = x;
    map[y.id] = y;
    EXPECT_EQ(map.at(x.id).id, x.id);
    EXPECT_EQ(map.at(y.id).id, y.id);
}
