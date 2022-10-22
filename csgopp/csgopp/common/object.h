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

struct ConstructorVisitor
{
    char* data;

    template<typename T>
    void operator()(const T& type) const { type.constructor(this->data); }
};

struct ArrayConstructorVisitor
{
    char* data;
    size_t length;
    size_t element_size;

    template<typename T>
    void operator()(const T& type) const
    {
        for (size_t i = 0; i < this->length; ++i)
        {
            type.constructor(this->data + this->element_size * i);
        }
    }
};

struct DestructorVisitor
{
    char* data;

    template<typename T>
    void operator()(const T& type) const { type.destructor(this->data); }
};

struct ArrayDestructorVisitor
{
    char* data;
    size_t length;
    size_t element_size;

    template<typename T>
    void operator()(const T& type) const
    {
        for (size_t i = 0; i < this->length; ++i)
        {
            type.destructor(this->data + this->element_size * (this->length - i - 1));
        }
    }
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

    void constructor(char* data) const
    {
        this->value.constructor(data);
    }

    void destructor(char* data) const
    {
        this->value.destructor(data);
    }

    void debug(size_t indent = 0) const;
};

struct PrototypeType
{
    struct Prototype* prototype;

    explicit PrototypeType(Prototype* prototype) : prototype(prototype) {}

    [[nodiscard]] size_t size() const;

    void constructor(char* data) const;
    void destructor(char* data) const;

    void debug(size_t indent = 0) const;
};

struct ArrayType;
using AnyType = std::variant<ValueType, PrototypeType, ArrayType>;

struct Member;

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
    [[nodiscard]] Member at(size_t index) const;

    void constructor(char* data) const;
    void destructor(char* data) const;

    void debug(size_t indent = 0) const;
};

struct Reference
{
    const AnyType* type;
    char* offset;
    size_t size;

    Reference(const AnyType* type, char* offset, size_t size)
        : type(type)
        , offset(offset)
        , size(size)
    {}

    Reference operator[](const std::string& name) const;
    Reference operator[](const size_t index) const;

    template<typename T>
    T& as()
    {
        return *reinterpret_cast<T*>(this->offset);
    }
};

struct Member
{
    const AnyType* type;
    size_t offset;
    size_t size;

    Member(const AnyType* type, size_t offset, size_t size)
        : type(type)
        , offset(offset)
        , size(size)
    {}

    Reference bind(char* instance) const;

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

    void constructor(char* data) const
    {
        std::visit(ConstructorVisitor{data + this->offset}, this->type);
    }

    void destructor(char* data) const
    {
        std::visit(DestructorVisitor{data + this->offset}, this->type);
    }

    [[nodiscard]] Member member() const
    {
        return {&this->type, this->offset, this->size};
    }
};

struct Instance
{
    const Prototype* prototype;
    char data[];

    explicit Instance(const Prototype* prototype);
    ~Instance();

    Reference operator[](const std::string& name);
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
    {}

    [[nodiscard]] Instance* allocate() const
    {
        char* data = new char[sizeof(Instance) + this->size];
        return new (data) Instance(this);
    }

    [[nodiscard]] Member at(const std::string& name) const  // tODO: want string view
    {
        return this->properties.at(this->lookup.at(name)).member();
    }

    void constructor(char* data) const
    {
        std::for_each(
            this->properties.begin(),
            this->properties.end(),
            [data](const Property& property) { property.constructor(data); });
    }

    void destructor(char* data) const
    {
        std::for_each(
            this->properties.rbegin(),
            this->properties.rend(),
            [data](const Property& property) { property.destructor(data); });
    }

    void debug(size_t indent = 0) const
    {
        using NamedMember = std::pair<std::string_view, Member>;
        std::vector<NamedMember> members;
        for (const std::pair<const std::string, size_t>& pair : this->lookup)
        {
            members.emplace_back(pair.first, this->properties[pair.second].member());
        }
        std::sort(members.begin(), members.end(), [](const NamedMember& a, const NamedMember& b)
        {
            return a.second.offset < b.second.offset;
        });
        for (const NamedMember& member : members)
        {
            std::cout << std::string(indent, ' ') << member.first;
            member.second.debug(indent + 2);
        }
    }
};

void PrototypeType::constructor(char* data) const
{
    this->prototype->constructor(data);
}

void PrototypeType::destructor(char* data) const
{
    this->prototype->destructor(data);
}

void ArrayType::constructor(char* data) const
{
    std::visit(ArrayConstructorVisitor{data, this->length, this->element_size}, *this->element_type);
}

void ArrayType::destructor(char* data) const
{
    std::visit(ArrayDestructorVisitor{data, this->length, this->element_size}, *this->element_type);
}

[[nodiscard]] size_t PrototypeType::size() const
{
    return this->prototype->size;
}

[[nodiscard]] size_t ArrayType::size() const
{
    return this->element_size * this->length;
}

Member ArrayType::at(size_t index) const
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
        Member member = this->at(i);
        std::cout << std::string(indent, ' ') << i;
        member.debug(indent + 2);
    }
}

Instance::Instance(const Prototype* prototype) : prototype(prototype)
{
    this->prototype->constructor(this->data);
}

Instance::~Instance()
{
    this->prototype->destructor(this->data);
}

Reference Member::bind(char* data) const
{
    return Reference(this->type, data + this->offset, this->size);
}

Reference Instance::operator[](const std::string& name)
{
    Member member = this->prototype->at(name);
    return member.bind(this->data);
}

void PrototypeType::debug(size_t indent) const
{
    this->prototype->debug(indent + 2);
}

Reference Reference::operator[](const std::string& name) const
{
    if (const PrototypeType* prototype_type = std::get_if<PrototypeType>(this->type))
    {
        return prototype_type->prototype->at(name).bind(this->offset);
    }

    throw std::runtime_error("reference is not an instance!");
}

Reference Reference::operator[](const size_t index) const
{
    if (const ArrayType* array_type = std::get_if<ArrayType>(this->type))
    {
        return array_type->at(index).bind(this->offset);
    }

    throw std::runtime_error("reference is not an array!");
}

}