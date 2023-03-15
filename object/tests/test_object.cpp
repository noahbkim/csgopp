#include <gtest/gtest.h>
#include <locale>
#include <codecvt>

#include <object/object.h>

using namespace object;

#define BOOL std::make_shared<TrivialValueType<bool>>()
#define UINT8 std::make_shared<TrivialValueType<uint8_t>>()
#define UINT32 std::make_shared<TrivialValueType<uint32_t>>()
#define STRING std::make_shared<TrivialValueType<std::string>>()

struct Vector3
{
    double x, y, z;
};

#define VECTOR std::make_shared<TrivialValueType<Vector3>>()

TEST(Object, integration)
{
    ObjectType::Builder entity_builder;
    entity_builder.member("id", UINT32);
    entity_builder.member("name", STRING);
    entity_builder.member("position", VECTOR);
    auto entity_type = Handle<ObjectType>::make(std::move(entity_builder));

    struct Entity
    {
        uint32_t id;
        std::string name;
        Vector3 position;
    };

    EXPECT_EQ(entity_type->at("id").offset, offsetof(Entity, id));
    EXPECT_EQ(entity_type->at("name").offset, offsetof(Entity, name));
    EXPECT_EQ(entity_type->at("position").offset, offsetof(Entity, position));
    EXPECT_EQ(entity_type->size(), sizeof(Entity));

    auto entity_array_type = Handle<ArrayType>::make(entity_type.get(), 2);
    EXPECT_EQ(entity_array_type->size(), sizeof(Entity) * 2);

    ObjectType::Builder engine_builder;
    engine_builder.member("alive", BOOL);
    engine_builder.member("flags", UINT32);
    engine_builder.member("entities", entity_array_type.get());
    auto engine_type = Handle<ObjectType>::make(engine_builder);

    struct Engine
    {
        bool alive{};
        uint32_t flags{};
        Entity entities[2];
    };

    EXPECT_TRUE(engine_type.view() == engine_type.view());
    EXPECT_TRUE(engine_type.view() <= engine_type.view());
    EXPECT_TRUE(engine_type.view() >= engine_type.view());
    EXPECT_TRUE(engine_type["alive"] <= engine_type.view());
    EXPECT_TRUE(engine_type["alive"] < engine_type.view());
    EXPECT_TRUE(engine_type.view() >= engine_type["alive"]);
    EXPECT_TRUE(engine_type.view() > engine_type["alive"]);
    EXPECT_EQ(engine_type->size(), sizeof(Engine));

    auto e = Object::make(engine_type);
    EXPECT_EQ(e["alive"].offset, 0);
    EXPECT_EQ(e["flags"].offset, 4);
    EXPECT_EQ(&e["alive"].is<bool>(), (bool*)e.data.get());
    EXPECT_EQ(&e["flags"].is<uint32_t>(), (uint32_t*)(e.data.get() + 4));
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

    EXPECT_EQ(engine_type["alive"](e).is<bool>(), true);
//    EXPECT_EQ(e[alive], true);
//    Is<std::string> entities_1_name = Accessor(engine_type)["entities"][1]["name"].is<std::string>();
//    EXPECT_EQ(entities_1_name(e), "dylan");
//    EXPECT_EQ(e[entities_1_name], "dylan");
//    EXPECT_THROW(Accessor a = Accessor(engine_type)["hello"], MemberError);
//    EXPECT_THROW(Accessor b = Accessor(engine_type)["entities"][2], IndexError);

//    std::shared_ptr<const Type> entities_array_T = e["entities"].type;
//    As<Entity> first_entity = Accessor(entities_array_T)[0].as<Entity>();
//    ASSERT_EQ(e["entities"][first_entity]->name, "noah");

    Engine* c = e.as<Engine>();
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
    auto object = Object::make(type);
    EXPECT_EQ(object.type, type);
}

TEST(Object, one_field_primitive)
{
    ObjectType::Builder builder;
    builder.member("value", UINT32);
    auto type = std::make_shared<ObjectType>(std::move(builder));
    EXPECT_EQ(type->size(), sizeof(uint32_t));
    auto object = Object::make(type);
    object["value"].is<uint32_t>() = 69;
    EXPECT_EQ(object["value"].is<uint32_t>(), 69);
}

TEST(Object, one_field_allocating)
{
    ObjectType::Builder builder;
    builder.member("value", STRING);
    auto type = std::make_shared<ObjectType>(std::move(builder));
    EXPECT_EQ(type->size(), sizeof(std::string));
    auto object = Object::make(type);
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
    ObjectType::Builder child_builder(parent);
    child_builder.member("third", STRING);
    auto child = std::make_shared<ObjectType>(std::move(child_builder));
    EXPECT_EQ(child->members.at(0).name, "first");
    EXPECT_EQ(child->members.at(1).name, "second");
    EXPECT_EQ(child->members.at(2).name, "third");
}

TEST(Object, type_lifetime)
{
    std::weak_ptr<ValueType> value_type_check;
    EXPECT_EQ(value_type_check.use_count(), 0);
    {
        auto value_type = std::make_shared<TrivialValueType<uint32_t>>();
        EXPECT_EQ(value_type.use_count(), 1);
        value_type_check = value_type;
        EXPECT_EQ(value_type.use_count(), 1);
        EXPECT_EQ(value_type_check.use_count(), 1);
    }
    EXPECT_TRUE(value_type_check.expired());
}

TEST(Object, instance_lifetime)
{
    std::weak_ptr<ValueType> value_type_check;
    EXPECT_EQ(value_type_check.use_count(), 0);
    {
        auto value_type = std::make_shared<TrivialValueType<uint32_t>>();
        EXPECT_EQ(value_type.use_count(), 1);  // value_T | value_T.self
        value_type_check = value_type;
        EXPECT_EQ(value_type.use_count(), 1);  // value_T | value_T.self, value_T_check
        EXPECT_EQ(value_type_check.use_count(), 1);

        {
            auto value = Value::make(value_type);
            EXPECT_EQ(value_type.use_count(), 2);  // value_T, value.type | value_T.self, value_T_check
            EXPECT_EQ(value_type_check.use_count(), 2);
        }

        EXPECT_EQ(value_type.use_count(), 1);  // value_T  // <-
        EXPECT_EQ(value_type_check.use_count(), 1);  // <-
    }
    EXPECT_TRUE(value_type_check.expired());  // <-
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
            std::shared_ptr<ValueType> value_type = std::make_shared<TrivialValueType<uint32_t>>();
            value_type_check = value_type;
            EXPECT_EQ(value_type.use_count(), 1);
            EXPECT_EQ(value_type_check.use_count(), 1);

            auto value = Value::make(value_type);
            EXPECT_EQ(value_type.use_count(), 2);
            EXPECT_EQ(value_type_check.use_count(), 2);

            value.is<uint32_t>() = 69;
            reference = Reference(value);
            ASSERT_EQ(reference.is<uint32_t>(), 69);
        }

        EXPECT_EQ(value_type_check.use_count(), 2);  // Reference (origin, type)
        EXPECT_EQ(value_type_check.use_count(), 2);
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
    builder.member("first", std::make_shared<TrivialValueType<T>>());
    builder.member("second", std::make_shared<TrivialValueType<U>>());
    builder.member("third", std::make_shared<TrivialValueType<V>>());
    ObjectType tripe_type(std::move(builder));
    using Actual = Triple<T, U, V>;
    ASSERT_EQ(tripe_type.at("first").offset, offsetof(Actual, first));
    ASSERT_EQ(tripe_type.at("second").offset, offsetof(Actual, second));
    ASSERT_EQ(tripe_type.at("third").offset, offsetof(Actual, third));
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
