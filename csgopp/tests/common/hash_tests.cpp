#include <gtest/gtest.h>

#include <csgopp/common/object.h>

using namespace csgopp::common::object;

TEST(Object, create)
{
    Prototype::Builder builder1;
    builder1.value("name", Value::of<std::string>());
    builder1.array("numbers", 2, ValueType(Value::of<int>()));
    Prototype foo(std::move(builder1));

    Prototype::Builder builder2;
    builder2.value("number", Value::of<int>());
    builder2.array("foos", 3, PrototypeType(&foo));
    Prototype bar(std::move(builder2));
    bar.debug();
}

TEST(Object, allocate)
{
    Prototype::Builder builder;
    builder.value("name", Value::of<std::string>());
    builder.array("numbers", 4, ValueType(Value::of<int>()));
    Prototype object(std::move(builder));
    object.debug();

    Instance* instance = object.allocate();
    std::string& name = (*instance)["name"].as<std::string>();
    name = "hello, world!";
    std::cout << (*instance)["name"].as<std::string>() << std::endl;

    delete instance;
}
