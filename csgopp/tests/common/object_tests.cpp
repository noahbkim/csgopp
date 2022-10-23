#include <gtest/gtest.h>

#include <csgopp/common/object.h>

std::shared_ptr<DefaultValueType<bool>> bool_T = std::make_shared<DefaultValueType<bool>>();
std::shared_ptr<DefaultValueType<uint8_t>> uint8_T = std::make_shared<DefaultValueType<uint8_t>>();
std::shared_ptr<DefaultValueType<uint32_t>> uint32_T = std::make_shared<DefaultValueType<uint32_t>>();
std::shared_ptr<DefaultValueType<std::string>> string_T = std::make_shared<DefaultValueType<std::string>>();

struct Vector3
{
    double x, y, z;
};

std::shared_ptr<DefaultValueType<Vector3>> vector3_T = std::make_shared<DefaultValueType<Vector3>>();

TEST(Object, integration)
{
    ObjectType::Builder entity_builder;
    entity_builder.member("id", uint32_T);
    entity_builder.member("name", string_T);
    entity_builder.member("position", vector3_T);
    std::shared_ptr<ObjectType> entity_T = std::make_unique<ObjectType>(std::move(entity_builder));

    struct Entity
    {
        uint32_t id;
        std::string name;
        Vector3 position;
    };

    ObjectType::Builder engine_builder;
    engine_builder.member("alive", bool_T);
    engine_builder.member("flags", uint32_T);
    engine_builder.member("entities", std::make_shared<ArrayType>(entity_T, 2));
    std::shared_ptr<ObjectType> engine_T = std::make_unique<ObjectType>(std::move(engine_builder));

    struct Engine
    {
        bool alive;
        uint32_t flags;
        Entity entities[2];
    };

    std::shared_ptr<Object> engine(instantiate(engine_T));
    Object& e = *engine;
    e["alive"].is<bool>() = true;
    e["flags"].is<uint32_t>() = 0xFF00FF00;
    e["entities"][0]["id"].is<uint32_t>() = 1;
    e["entities"][0]["name"].is<std::string>() = "noah";
    e["entities"][1]["id"].is<uint32_t>() = 2;
    e["entities"][1]["name"].is<std::string>() = "dylan";

    EXPECT_EQ(e["alive"].is<bool>(), true);
    EXPECT_EQ(e["flags"].is<uint32_t>(), 0xFF00FF00);
    EXPECT_EQ(e["entities"][0]["id"].is<uint32_t>(), 1);
    EXPECT_EQ(e["entities"][0]["name"].is<std::string>(), "noah");
    EXPECT_EQ(e["entities"][1]["id"].is<uint32_t>(), 2);
    EXPECT_EQ(e["entities"][1]["name"].is<std::string>(), "dylan");

    Is<bool> alive = (*engine_T)["alive"].is<bool>();
    EXPECT_EQ(alive(e), true);
    Is<std::string> entities_1_name = (*engine_T)["entities"][1]["name"].is<std::string>();
    EXPECT_EQ(entities_1_name(e), "dylan");

    Engine* c{e.as<Engine>()};
    EXPECT_EQ(c->alive, true);
    EXPECT_EQ(c->flags, 0xFF00FF00);
    EXPECT_EQ(c->entities[0].id, 1);
    EXPECT_EQ(c->entities[0].name, "noah");
    EXPECT_EQ(c->entities[1].id, 2);
    EXPECT_EQ(c->entities[1].name, "dylan");
}

template<typename T, typename U, typename V>
struct Triple
{
    T first;
    U second;
    V third;
};

template<typename T, typename U, typename V>
void test_alignment()
{
    ObjectType::Builder builder;
    builder.member("first", std::make_unique<DefaultValueType<T>>());
    builder.member("second", std::make_unique<DefaultValueType<U>>());
    builder.member("third", std::make_unique<DefaultValueType<V>>());
    ObjectType tripe_T(std::move(builder));
    using Actual = Triple<T, U, V>;
    ASSERT_EQ(tripe_T.at("first").offset, offsetof(Actual, first));
    ASSERT_EQ(tripe_T.at("second").offset, offsetof(Actual, second));
    ASSERT_EQ(tripe_T.at("third").offset, offsetof(Actual, third));
}

#define TRIPLE(T, U, V) Triple<T, U, V>
#define TEST_ALIGNMENT_NAMED(T, U, V, N) TEST(Object, align_##N) { test_alignment<T, U, V>(); }
#define TEST_ALIGNMENT(T, U, V) TEST_ALIGNMENT_NAMED(T, U, V, T##_##U##_##V)

TEST_ALIGNMENT(uint8_t, uint8_t, uint8_t);
TEST_ALIGNMENT(uint16_t, uint8_t, uint8_t);
TEST_ALIGNMENT(uint32_t, uint8_t, uint8_t);
TEST_ALIGNMENT(uint64_t, uint8_t, uint8_t);
TEST_ALIGNMENT(uint8_t, uint16_t, uint8_t);
TEST_ALIGNMENT(uint8_t, uint32_t, uint8_t);
TEST_ALIGNMENT(uint8_t, uint64_t, uint8_t);
TEST_ALIGNMENT(uint16_t, uint16_t, uint8_t);
TEST_ALIGNMENT(uint32_t, uint32_t, uint8_t);
TEST_ALIGNMENT(uint64_t, uint64_t, uint8_t);
TEST_ALIGNMENT_NAMED(uint8_t, TRIPLE(uint8_t, uint8_t, uint8_t), uint8_t, uint8_t_struct_uint8_t);
TEST_ALIGNMENT_NAMED(uint16_t, TRIPLE(uint8_t, uint8_t, uint8_t), uint8_t, uint16_t_struct_uint8_t);
TEST_ALIGNMENT_NAMED(uint32_t, TRIPLE(uint8_t, uint8_t, uint8_t), uint8_t, uint32_t_struct_uint8_t);
TEST_ALIGNMENT_NAMED(uint64_t, TRIPLE(uint8_t, uint8_t, uint8_t), uint8_t, uint64_t_struct_uint8_t);
TEST_ALIGNMENT_NAMED(uint8_t, TRIPLE(uint16_t, uint8_t, uint8_t), uint8_t, uint8_t_struct_uint16_t);
TEST_ALIGNMENT_NAMED(uint8_t, TRIPLE(uint32_t, uint8_t, uint8_t), uint8_t, uint8_t_struct_uint32_t);
TEST_ALIGNMENT_NAMED(uint8_t, TRIPLE(uint64_t, uint8_t, uint8_t), uint8_t, uint8_t_struct_uint64_t);