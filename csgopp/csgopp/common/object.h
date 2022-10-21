#pragma once

#include <string>
#include <unordered_map>
#include <iostream>

namespace csgopp::common::object
{

struct Member
{
    virtual ~Member() = default;
};

template<typename T>
struct PrimitiveMember final : public Member
{
    T value;
};

struct Property
{
    virtual ~Property() = default;

    [[nodiscard]] virtual size_t size() const = 0;

    virtual Member* construct(char* blob) const = 0;
    virtual void destroy(Member* member) const = 0;
};

template<typename M>
struct PropertyBase : public Property
{
    using MemberType = M;

    Member* construct(char* blob) const override
    {
        auto* as = reinterpret_cast<M*>(blob);
        new (as) M();
        return as;
    }

    void destroy(Member* member) const override
    {
        auto* as = dynamic_cast<M*>(member);
        as->~M();
    }
};

template<typename T>
struct PrimitiveProperty : public PropertyBase<PrimitiveMember<T>>
{
    using MemberType = typename PropertyBase<PrimitiveMember<T>>::MemberType;

    [[nodiscard]] size_t size() const override
    {
        return sizeof(MemberType);
    }
};

struct Structure;

struct Object {
    const Structure* structure;
    std::unordered_map<std::string, Member*> members;

    Object(const Structure* structure) : structure(structure) {}
    Object(const Object& other) = delete;
    ~Object();

private:
    friend Structure;

    Object();
};

struct Structure {
    std::unordered_map<std::string, Property*> properties;

    ~Structure() {
        for (const auto& [name, property] : this->properties) {
            delete property;
        }
    }

    Object* allocate() const {
        size_t needs = sizeof(Object);
        std::cout << "object " << needs << std::endl;
        for (const auto& [name, property] : this->properties) {
            std::cout << "  " << name << " " << property->needs() << std::endl;
            needs += property->needs();
        }

        char* blob = new char[needs];
        Object* object = reinterpret_cast<Object*>(blob);
        new (object) Object(this);
        blob += sizeof(Object);

        for (const auto& [name, property] : this->properties) {
            Member* member = property->construct(blob);
            object->members.emplace(name, member);
            blob += property->needs();
        }

        return object;
    }
};

Object::Object() {}

Object::~Object() {
    std::cerr << "destroying" << std::endl;
    for (const auto& [name, property] : this->structure->properties) {
        property->destroy(this->members.at(name));
    }
}

using IntProperty = PrimitiveProperty<int>;

//int main() {
//    Structure pair;
//    pair.properties.emplace("a", new IntProperty());
//    pair.properties.emplace("b", new IntProperty());
//
//    Object* object = pair.allocate();
//    dynamic_cast<IntProperty::M*>(object->members["a"])->value = 10;
//    dynamic_cast<IntProperty::M*>(object->members["b"])->value = 4710;
//    std::cout << dynamic_cast<IntProperty::M*>(object->members["a"])->value << ", " << dynamic_cast<IntProperty::M*>(object->members["b"])->value << std::endl;
//
//    delete object;
//}

}