#include <gtest/gtest.h>
#include <locale>
#include <codecvt>

#include <object/object.h>

using namespace object;

template<typename T>
struct TestDefaultValueType : public DefaultValueType<T>
{
    void format(const char* address, std::ostream& out) const override
    {

    }
};

#define BOOL std::make_shared<TestDefaultValueType<bool>>()
#define UINT8 std::make_shared<TestDefaultValueType<uint8_t>>()
#define UINT32 std::make_shared<TestDefaultValueType<uint32_t>>()
#define STRING std::make_shared<TestDefaultValueType<std::string>>()

struct Vector3
{
    double x, y, z;
};

#define VECTOR std::make_shared<TestDefaultValueType<Vector3>>()

TEST(Object, integration)
{
    ObjectType::Builder entity_builder;
    entity_builder.member("id", UINT32);
    entity_builder.member("name", STRING);
    entity_builder.member("position", VECTOR);
    std::shared_ptr<ObjectType> entity_T = std::make_shared<ObjectType>(std::move(entity_builder));

    struct Entity
    {
        uint32_t id;
        std::string name;
        Vector3 position;
    };

    EXPECT_EQ(entity_T->at("id").offset, offsetof(Entity, id));
    EXPECT_EQ(entity_T->at("name").offset, offsetof(Entity, name));
    EXPECT_EQ(entity_T->at("position").offset, offsetof(Entity, position));
    EXPECT_EQ(entity_T->size(), sizeof(Entity));

    auto entity_array_T = std::make_shared<ArrayType>(entity_T, 2);
    EXPECT_EQ(entity_array_T->size(), sizeof(Entity) * 2);

    ObjectType::Builder engine_builder;
    engine_builder.member("alive", BOOL);
    engine_builder.member("flags", UINT32);
    engine_builder.member("entities", entity_array_T);
    auto engine_T = std::make_shared<ObjectType>(std::move(engine_builder));

    EXPECT_TRUE(Accessor(engine_T).is_equal(Accessor(engine_T)));
    EXPECT_TRUE(Accessor(engine_T).is_subset_of(Accessor(engine_T)));
    EXPECT_TRUE(Accessor(engine_T).is_superset_of(Accessor(engine_T)));
    EXPECT_TRUE(Accessor(engine_T)["alive"].is_subset_of(Accessor(engine_T)));
    EXPECT_TRUE(Accessor(engine_T)["alive"].is_strict_subset_of(Accessor(engine_T)));
    EXPECT_TRUE(Accessor(engine_T).is_superset_of(Accessor(engine_T)["alive"]));
    EXPECT_TRUE(Accessor(engine_T).is_strict_superset_of(Accessor(engine_T)["alive"]));

    struct Engine
    {
        bool alive{};
        uint32_t flags{};
        Entity entities[2];
    };

    Object e(engine_T);
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
    EXPECT_THROW(e["hello"], MemberError);
    EXPECT_THROW(e["entities"][2], IndexError);

    Is<bool> alive = Accessor(engine_T)["alive"].is<bool>();
    EXPECT_EQ(alive(e), true);
    EXPECT_EQ(e[alive], true);
    Is<std::string> entities_1_name = Accessor(engine_T)["entities"][1]["name"].is<std::string>();
    EXPECT_EQ(entities_1_name(e), "dylan");
    EXPECT_EQ(e[entities_1_name], "dylan");
    EXPECT_THROW(Accessor a = Accessor(engine_T)["hello"], MemberError);
    EXPECT_THROW(Accessor b = Accessor(engine_T)["entities"][2], IndexError);

    std::shared_ptr<const Type> entities_array_T = e["entities"].type;
    As<Entity> first_entity = Accessor(entities_array_T)[0].as<Entity>();
    ASSERT_EQ(e["entities"][first_entity]->name, "noah");

    Engine* c{e.as<Engine>()};
    EXPECT_EQ(c->alive, true);
    EXPECT_EQ(c->flags, 0xFF00FF00);
    EXPECT_EQ(c->entities[0].id, 1);
    EXPECT_EQ(c->entities[0].name, "noah");
    EXPECT_EQ(c->entities[1].id, 2);
    EXPECT_EQ(c->entities[1].name, "dylan");
}

TEST(Object, null)
{
    ObjectType::Builder builder;
    auto type = std::make_shared<ObjectType>(std::move(builder));
    EXPECT_EQ(type->size(), 0);
    Object object(type);
    EXPECT_EQ(object.type, type);
}

TEST(Object, one_field_primitive)
{
    ObjectType::Builder builder;
    builder.member("value", UINT32);
    auto type = std::make_shared<ObjectType>(std::move(builder));
    EXPECT_EQ(type->size(), sizeof(uint32_t));
    Object object(type);
    object["value"].is<uint32_t>() = 69;
    EXPECT_EQ(object["value"].is<uint32_t>(), 69);
}

TEST(Object, one_field_allocating)
{
    ObjectType::Builder builder;
    builder.member("value", STRING);
    auto type = std::make_shared<ObjectType>(std::move(builder));
    EXPECT_EQ(type->size(), sizeof(std::string));
    Object object(type);
    EXPECT_EQ(object["value"].is<std::string>(), "");
    object["value"].is<std::string>() = "hello, world!";
    EXPECT_EQ(object["value"].is<std::string>(), "hello, world!");
}

TEST(Object, inherit_simple)
{
    ObjectType::Builder parent_builder;
    parent_builder.member("first", UINT8);
    parent_builder.member("second", UINT32);
    auto parent = std::make_shared<ObjectType>(std::move(parent_builder));
    ObjectType::Builder child_builder(parent.get());
    child_builder.member("third", STRING);
    auto child = std::make_shared<ObjectType>(std::move(child_builder));
    EXPECT_EQ(child->members.at(0).name, "first");
    EXPECT_EQ(child->members.at(1).name, "second");
    EXPECT_EQ(child->members.at(2).name, "third");
}

TEST(Object, type_lifetime)
{
    std::weak_ptr<ValueType> value_T_check;
    EXPECT_EQ(value_T_check.use_count(), 0);
    {
        auto value_T = std::make_shared<TestDefaultValueType<uint32_t>>();
        EXPECT_EQ(value_T.use_count(), 1);
        value_T_check = value_T;
        EXPECT_EQ(value_T.use_count(), 1);
        EXPECT_EQ(value_T_check.use_count(), 1);
    }
    EXPECT_TRUE(value_T_check.expired());
}

TEST(Object, instance_lifetime)
{
    std::weak_ptr<ValueType> value_T_check;
    EXPECT_EQ(value_T_check.use_count(), 0);
    {
        auto value_T = std::make_shared<TestDefaultValueType<uint32_t>>();
        EXPECT_EQ(value_T.use_count(), 1);  // value_T | value_T.self
        value_T_check = value_T;
        EXPECT_EQ(value_T.use_count(), 1);  // value_T | value_T.self, value_T_check
        EXPECT_EQ(value_T_check.use_count(), 1);

        {
            Value value(value_T);
            EXPECT_EQ(value_T.use_count(), 2);  // value_T, value.type | value_T.self, value_T_check
            EXPECT_EQ(value_T_check.use_count(), 2);
        }

        EXPECT_EQ(value_T.use_count(), 1);  // value_T  // <-
        EXPECT_EQ(value_T_check.use_count(), 1);  // <-
    }
    EXPECT_TRUE(value_T_check.expired());  // <-
}

TEST(Object, reference_lifetime)
{
    // This should not persist the value
    std::weak_ptr<ValueType> value_type_check;
    {
        // Points to nothing by default
        Reference reference;
        {
            // Has to be anonymous
            std::shared_ptr<ValueType> value_T = std::make_shared<TestDefaultValueType<uint32_t>>();
            value_type_check = value_T;
            EXPECT_EQ(value_T.use_count(), 1);
            EXPECT_EQ(value_type_check.use_count(), 1);

            Value value(value_T);
            EXPECT_EQ(value_T.use_count(), 2);
            EXPECT_EQ(value_type_check.use_count(), 2);

            value.is<uint32_t>() = 69;
            reference = *value;
            ASSERT_EQ(reference.is<uint32_t>(), 69);
        }

        EXPECT_EQ(value_type_check.use_count(), 1);  // Reference
        EXPECT_EQ(value_type_check.use_count(), 1);
        uint32_t check = reference.is<uint32_t>();
        ASSERT_EQ(check, 69);
        EXPECT_FALSE(value_type_check.expired());
    }
    EXPECT_TRUE(value_type_check.expired());
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
    builder.member("first", std::make_shared<TestDefaultValueType<T>>());
    builder.member("second", std::make_shared<TestDefaultValueType<U>>());
    builder.member("third", std::make_shared<TestDefaultValueType<V>>());
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
TEST_ALIGNMENT_NAMED(uint8_t, std::string, uint8_t, uint8_t_std_string_uint8_t);
TEST_ALIGNMENT_NAMED(uint16_t, std::string, uint8_t, uint16_t_std_string_uint8_t);
TEST_ALIGNMENT_NAMED(uint32_t, std::string, uint8_t, uint32_t_std_string_uint8_t);
TEST_ALIGNMENT_NAMED(uint64_t, std::string, uint8_t, uint64_t_std_string_uint8_t);
TEST_ALIGNMENT_NAMED(uint8_t, TRIPLE(uint8_t, uint8_t, uint8_t), uint8_t, uint8_t_struct_uint8_t);
TEST_ALIGNMENT_NAMED(uint16_t, TRIPLE(uint8_t, uint8_t, uint8_t), uint8_t, uint16_t_struct_uint8_t);
TEST_ALIGNMENT_NAMED(uint32_t, TRIPLE(uint8_t, uint8_t, uint8_t), uint8_t, uint32_t_struct_uint8_t);
TEST_ALIGNMENT_NAMED(uint64_t, TRIPLE(uint8_t, uint8_t, uint8_t), uint8_t, uint64_t_struct_uint8_t);
TEST_ALIGNMENT_NAMED(uint8_t, TRIPLE(uint16_t, uint8_t, uint8_t), uint8_t, uint8_t_struct_uint16_t);
TEST_ALIGNMENT_NAMED(uint8_t, TRIPLE(uint32_t, uint8_t, uint8_t), uint8_t, uint8_t_struct_uint32_t);
TEST_ALIGNMENT_NAMED(uint8_t, TRIPLE(uint64_t, uint8_t, uint8_t), uint8_t, uint8_t_struct_uint64_t);
