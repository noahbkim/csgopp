#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <absl/container/flat_hash_map.h>
#include <iostream>

namespace csgopp::common::object {

struct SizeVisitor
{
    template<typename T>
    size_t operator()(const T& type) const { return type.size(); }
};

struct DebugVisitor
{
    size_t indent;

    template<typename T>
    void operator()(const T& type) const { type.debug(this->indent); }
};

struct Value
{
    typedef void (*Constructor)(char*);
    typedef void (*Destructor)(char*);

    size_t size;
    Constructor constructor;
    Destructor destructor;

    Value(size_t size, Constructor constructor, Destructor destructor)
        : size(size)
        , constructor(constructor)
        , destructor(destructor)
    {}

    template<typename T>
    static Value of()
    {
        return Value(sizeof(T), &Value::construct<T>, &Value::destroy<T>);
    }

    template<typename T>
    static void construct(char* address)
    {
        new (reinterpret_cast<T*>(address)) T;
    }

    template<typename T>
    static void destroy(char* address)
    {
        reinterpret_cast<T*>(address)->~T();
    }
};

struct ValueType
{
    Value value;

    explicit ValueType(const Value& value) : value(value) {}
    ValueType(ValueType&& other) noexcept : value(std::move(other.value)) {}
    virtual ~ValueType() = default;

    [[nodiscard]] size_t size() const
    {
        return this->value.size;
    }

    void debug(size_t indent = 0) const;
};

struct PrototypeType
{
    struct Prototype* prototype;

    explicit PrototypeType(Prototype* prototype) : prototype(prototype) {}

    [[nodiscard]] size_t size() const;

    void debug(size_t indent = 0) const;
};

struct ArrayType;
using AnyType = std::variant<ValueType, PrototypeType, ArrayType>;

struct Reference;

struct ArrayType
{
    size_t length;
    std::unique_ptr<AnyType> element_type;
    size_t element_size;

    template<typename... Args>
    explicit ArrayType(size_t length, Args&&... args)
        : length(length)
        , element_type(std::make_unique<AnyType>(std::forward<Args>(args)...))
        , element_size(std::visit(SizeVisitor{}, *this->element_type)) {}

    ArrayType(ArrayType&& other) noexcept
        : length(other.length)
        , element_type(std::move(other.element_type))
        , element_size(other.element_size) {}

    [[nodiscard]] size_t size() const;
    [[nodiscard]] Reference at(size_t index) const;

    void debug(size_t indent = 0) const;
};

struct Reference
{
    const AnyType* type;
    size_t offset;
    size_t size;

    Reference(const AnyType* type, size_t offset, size_t size)
        : type(type)
        , offset(offset)
        , size(size)
    {}

    void debug(size_t indent = 0) const
    {
        std::cout << " (offset: " << this->offset << ", size: " << this->size << ")" << std::endl;
        std::visit(DebugVisitor{indent + 2}, *this->type);
    }
};

struct Property
{
    AnyType type;
    size_t offset;
    size_t size;

    template<typename... Args>
    explicit Property(size_t offset, Args&&... args)
        : type(std::forward<Args>(args)...)
        , offset(offset)
        , size(std::visit(SizeVisitor{}, this->type))
    {}

    [[nodiscard]] Reference reference() const
    {
        return {&this->type, this->offset, this->size};
    }
};

struct Prototype
{
    struct Builder
    {
        size_t size{};
        std::vector<Property> properties;
        absl::flat_hash_map<std::string, size_t> lookup;

        template<typename Name, typename... Args>
        void add(Name&& name, Args&&... args)
        {
            const Property& property = this->properties.emplace_back(
                this->size,
                std::forward<Args>(args)...);
            this->size += property.size;
            this->lookup.emplace(std::forward<Name>(name), this->properties.size() - 1);
        }

        template<typename Name>
        void value(Name&& name, Value value)
        {
            this->add(
                std::forward<Name>(name),
                std::move(ValueType(value)));
        }

        template<typename Name, typename... Args>
        void array(Name&& name, size_t length, Args&&... args)
        {
            this->add(
                std::forward<Name>(name),
                std::move(ArrayType(length, std::forward<Args>(args)...)));
        }

        template<typename Name>
        void prototype(Name&& name, Prototype* prototype)
        {
            this->add(
                std::forward<Name>(name),
                std::move(PrototypeType(prototype)));
        }
    };

    size_t size;
    std::vector<Property> properties;
    absl::flat_hash_map<std::string, size_t> lookup;

    explicit Prototype(Builder&& builder)
        : size(builder.size)
        , properties(std::move(builder.properties))
        , lookup(std::move(builder.lookup))
    {
    }

    void debug(size_t indent = 0) const
    {
        using Member = std::pair<std::string_view, Reference>;
        std::vector<Member> members;
        for (const std::pair<const std::string, size_t>& pair : this->lookup)
        {
            members.emplace_back(pair.first, this->properties[pair.second].reference());
        }
        std::sort(members.begin(), members.end(), [](const Member& a, const Member& b)
        {
            return a.second.offset < b.second.offset;
        });
        for (const Member& member : members)
        {
            std::cout << std::string(indent, ' ') << member.first;
            member.second.debug(indent + 2);
        }
    }
};

[[nodiscard]] size_t PrototypeType::size() const
{
    return this->prototype->size;
}

[[nodiscard]] size_t ArrayType::size() const
{
    return this->element_size * this->length;
}

Reference ArrayType::at(size_t index) const
{
    return {this->element_type.get(), this->element_size * index, this->element_size};
}

void ValueType::debug(size_t indent) const
{

}

void ArrayType::debug(size_t indent) const
{
    for (size_t i = 0; i < this->length; ++i)
    {
        Reference reference = this->at(i);
        std::cout << std::string(indent, ' ') << i;
        reference.debug(indent + 2);
    }
}

void PrototypeType::debug(size_t indent) const
{
    this->prototype->debug(indent + 2);
}

}