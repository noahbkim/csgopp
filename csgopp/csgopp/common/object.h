#pragma once

#include <string>
#include <utility>
#include <vector>
#include <variant>
#include <typeinfo>
#include <algorithm>
#include <stdexcept>
#include <absl/container/flat_hash_map.h>

struct Error : std::runtime_error
{
    using std::runtime_error::runtime_error;
};

struct MemberError : Error
{
    using Error::Error;
};

struct IndexError : Error
{
    using Error::Error;
};

struct TypeError : Error
{
    using Error::Error;
};

template<typename T>
static constexpr size_t alignmentof()
{
    struct Measure { uint8_t bump; T aligned; };
    return offsetof(Measure, aligned);
}

static constexpr size_t align(size_t offset, size_t alignment)
{
    if (alignment == 0)
    {
        return 0;
    }
    else
    {
        return (offset + alignment - 1) / alignment * alignment;
    }
}

template<typename T>
struct Instance;

struct Reference;

struct Type;

template<typename T>
struct As
{
    const Type* origin;
    size_t offset;

    As(const Type* origin, size_t offset);

    T* operator()(Reference& reference) const;

    template<typename U>
    T* operator()(Instance<U>& instance) const;

private:
    T* bind(char* address) const;
};

template<typename T>
struct Is
{
    const Type* origin;
    size_t offset;

    Is(const Type* origin, size_t offset);

    template<typename U>
    T& operator()(Reference& reference) const;

    template<typename U>
    T& operator()(Instance<U>& instance) const;

private:
    T& bind(char* address) const;
};

struct Accessor
{
    const struct Type* origin;
    const struct Type* type;
    size_t offset;

    Accessor(const struct Type* origin, const struct Type* type, size_t offset);

    Accessor operator[](const std::string& member_name) const;
    Accessor operator[](size_t element_index) const;

    template<typename T>
    Reference operator()(Instance<T>& instance) const;

    template<typename T>
    As<T> as() const;

    template<typename T>
    Is<T> is() const;

private:
    Reference bind(char* address) const;
};

struct Type
{
    virtual ~Type() = default;

    [[nodiscard]] virtual size_t size() const = 0;
    [[nodiscard]] virtual size_t alignment() const = 0;

    virtual void construct(char* address) const = 0;
    virtual void destroy(char* address) const = 0;

    [[nodiscard]] virtual Accessor operator[](const std::string& member_name) const;
    [[nodiscard]] virtual Accessor operator[](size_t element_index) const;
};

struct Reference
{
    const Type* type;
    char* address;

    Reference(const Type* type, char* address);

    Reference operator[](const std::string& member_name) const;
    Reference operator[](size_t element_index) const;

    template<typename T>
    T* operator[](const As<T>& as);

    template<typename T>
    T& operator[](const Is<T>& as);

    template<typename T>
    T* as();

    template<typename T>
    T& is();
};

struct ValueType : public Type
{
    [[nodiscard]] virtual const std::type_info& info() const = 0;
};

template<typename T>
struct DefaultValueType final : public ValueType
{
    [[nodiscard]] size_t size() const override;
    [[nodiscard]] size_t alignment() const override;
    [[nodiscard]] const std::type_info& info() const override;

    void construct(char* address) const override;
    void destroy(char* address) const override;
};

struct ArrayType final : public Type
{
    std::shared_ptr<const Type> element_type;
    size_t element_size;
    size_t length;

    ArrayType(std::shared_ptr<const Type> element_type, size_t length);
    [[nodiscard]] size_t size() const override;
    [[nodiscard]] size_t alignment() const override;

    void construct(char* address) const override;
    void destroy(char* address) const override;

    [[nodiscard]] Accessor operator[](size_t element_index) const override;

    [[nodiscard]] size_t at(size_t element_index) const;
};

struct Member
{
    std::shared_ptr<const Type> type;
    size_t offset;

    Member(std::shared_ptr<const Type> type, size_t offset);
};

struct ObjectType final : public Type
{
    using Members = std::vector<Member>;
    using MembersLookup = absl::flat_hash_map<std::string, size_t>;

    struct Builder
    {
        Members members;
        MembersLookup members_lookup;
        size_t members_size = 0;
        std::string name;

        Builder() = default;
        explicit Builder(std::string name);

        void member(const std::string& member_name, std::shared_ptr<const Type> type);
    };

    Members members;
    MembersLookup members_lookup;
    size_t members_size;
    std::string name;

    explicit ObjectType(Builder&& builder);
    [[nodiscard]] size_t size() const override;
    [[nodiscard]] size_t alignment() const override;

    void construct(char* address) const override;
    void destroy(char* address) const override;

    [[nodiscard]] Accessor operator[](const std::string& member_name) const override;

    [[nodiscard]] const Member& at(const std::string& member_name) const;
};

template<typename T>
struct Instance
{
    const T* type;
    char address[];

    ~Instance();
    Instance(Instance& other) = delete;
    Instance(Instance&& other) = delete;
    Instance& operator=(Instance& other) = delete;
    Instance& operator=(Instance&& other) = delete;

    Reference operator[](const std::string& member_name);
    Reference operator[](size_t element_index);
    Reference operator*();

    template<typename U>
    U* operator[](const As<U>& as);

    template<typename U>
    U& operator[](const Is<U>& as);

    template<typename U>
    U* as();

    template<typename U>
    typename std::enable_if<std::is_base_of<T, ValueType>::value, U>::type& is();

private:
    template<typename U>
    friend Instance<U>* instantiate(std::shared_ptr<const U> type);

    template<typename U>
    friend Instance<U>* instantiate(std::shared_ptr<U> type);

    template<typename U>
    friend Instance<U>* instantiate(const U* type);

    explicit Instance(const T* type);
};

using Object = Instance<ObjectType>;
using Array = Instance<ArrayType>;

template<typename T>
Instance<T>* instantiate(std::shared_ptr<const T> type)
{
    char* address = new char[sizeof(Instance<T>) + type->size()];
    return new (address) Instance<T>(type.get());
}

template<typename T>
Instance<T>* instantiate(std::shared_ptr<T> type)
{
    char* address = new char[sizeof(Instance<T>) + type->size()];
    return new (address) Instance<T>(type.get());
}

template<typename T>
Instance<T>* instantiate(const T* type)
{
    char* address = new char[sizeof(Instance<T>) + type->size()];
    return new (address) Instance<T>(type);
}

template<typename T>
As<T>::As(const Type* origin, size_t offset)
    : origin(origin)
    , offset(offset)
{}

template<typename T>
T* As<T>::operator()(Reference& reference) const
{
    if (reference.type == this->origin)
    {
        return this->bind(reference.address);
    }
    else
    {
        throw TypeError("accessor target does not match its origin");
    }
}

template<typename T>
template<typename U>
T* As<T>::operator()(Instance<U>& instance) const
{
    if (instance.type == this->origin)
    {
        return this->bind(instance.address);
    }
    else
    {
        throw TypeError("accessor target does not match its origin");
    }
}

template<typename T>
T* As<T>::bind(char* address) const
{
    return reinterpret_cast<T*>(address + this->offset);
}

template<typename T>
Is<T>::Is(const Type* origin, size_t offset)
    : origin(origin)
    , offset(offset)
{}

template<typename T>
template<typename U>
T& Is<T>::operator()(Reference& reference) const
{
    if (reference.type == this->origin)
    {
        return this->bind(reference.address);
    }
    else
    {
        throw TypeError("accessor target does not match its origin");
    }
}

template<typename T>
template<typename U>
T& Is<T>::operator()(Instance<U>& instance) const
{
    if (instance.type == this->origin)
    {
        return this->bind(instance.address);
    }
    else
    {
        throw TypeError("accessor target does not match its origin");
    }
}

template<typename T>
T& Is<T>::bind(char* address) const
{
    return *reinterpret_cast<T*>(address + this->offset);
}

template<typename T>
Reference Accessor::operator()(Instance<T>& instance) const
{
    return this->bind(instance.address);
}

template<typename T>
As<T> Accessor::as() const
{
    return As<T>(this->origin, this->offset);
}

template<typename T>
Is<T> Accessor::is() const
{
    auto* value_type = dynamic_cast<const ValueType*>(this->type);
    if (value_type != nullptr)
    {
        if (value_type->info() == typeid(T))
        {
            return Is<T>(this->origin, this->offset);
        }
        else
        {
            throw TypeError("cast does not match original type!");
        }
    }
    else
    {
        throw TypeError("cast is only available for values!");
    }
}

Reference Accessor::bind(char* address) const
{
    return Reference(this->type, address + this->offset);
}

[[nodiscard]] Accessor Type::operator[](const std::string& member_name) const
{
    throw TypeError("member access is only available on objects!");
}

[[nodiscard]] Accessor Type::operator[](size_t element_index) const
{
    throw TypeError("indexing is only available on arrays!");
}

Reference::Reference(const Type* type, char* address)
    : type(type)
    , address(address)
{}

Reference Reference::operator[](const std::string& member_name) const
{
    auto* object_type = dynamic_cast<const ObjectType*>(this->type);
    if (object_type != nullptr)
    {
        const Member& member = object_type->at(member_name);
        return Reference(member.type.get(), this->address + member.offset);
    }
    else
    {
        throw TypeError("member access is only available on objects!");
    }
}

Reference Reference::operator[](size_t element_index) const
{
    auto* array_type = dynamic_cast<const ArrayType*>(this->type);
    if (array_type != nullptr)
    {
        size_t element_offset = array_type->at(element_index);
        return Reference(array_type->element_type.get(), this->address + element_offset);
    }
    else
    {
        throw TypeError("indexing is only available on arrays!");
    }
}

template<typename T>
void leak(T* address) {}

template<typename T>
std::shared_ptr<T> make_shared_static()
{
    static T type;
    return std::shared_ptr<T>(&type, &leak<T>);
}

template<typename T>
T* Reference::operator[](const As<T>& as)
{
    return as(*this);
}

template<typename T>
T& Reference::operator[](const Is<T>& is)
{
    return is(*this);
}

template<typename T>
T* Reference::as()
{
    return reinterpret_cast<T*>(this->address);
}

template<typename T>
T& Reference::is()
{
    auto* value_type = dynamic_cast<const ValueType*>(this->type);
    if (value_type != nullptr)
    {
        if (value_type->info() == typeid(T))
        {
            return *reinterpret_cast<T*>(this->address);
        }
        else
        {
            throw TypeError("cast does not match original type!");
        }
    }
    else
    {
        throw TypeError("cast is only available for values!");
    }
}

Accessor::Accessor(const Type* origin, const Type* type, size_t offset)
    : origin(origin)
    , type(type)
    , offset(offset)
{}

Accessor Accessor::operator[](const std::string& member_name) const
{
    auto* object_type = dynamic_cast<const ObjectType*>(this->type);
    if (object_type != nullptr)
    {
        const Member& member = object_type->at(member_name);
        return Accessor(this->origin, member.type.get(), this->offset + member.offset);
    }
    else
    {
        throw TypeError("member access is only available on objects!");
    }
}

Accessor Accessor::operator[](size_t element_index) const
{
    auto* array_type = dynamic_cast<const ArrayType*>(this->type);
    if (array_type != nullptr)
    {
        size_t element_offset = array_type->at(element_index);
        return Accessor(this->origin, array_type->element_type.get(), this->offset + element_offset);
    }
    else
    {
        throw TypeError("indexing is only available on arrays!");
    }
}

template<typename T>
[[nodiscard]] size_t DefaultValueType<T>::size() const
{
    return sizeof(T);
}

template<typename T>
[[nodiscard]] size_t DefaultValueType<T>::alignment() const
{
    return alignmentof<T>();
}

template<typename T>
[[nodiscard]] const std::type_info& DefaultValueType<T>::info() const
{
    return typeid(T);
}

template<typename T>
void DefaultValueType<T>::construct(char* address) const
{
    if constexpr (std::is_constructible<T>::value)
    {
        printf("constructing %s %p\n", this->info().name(), address);
        new (address) T;
    }
}

template<typename T>
void DefaultValueType<T>::destroy(char* address) const
{
    if constexpr (std::is_destructible<T>::value)
    {
        reinterpret_cast<T*>(address)->~T();
    }
}

ArrayType::ArrayType(std::shared_ptr<const Type> element_type, size_t length)
    : element_type(std::move(element_type))
    , element_size(align(this->element_type->alignment(), this->element_type->size()))
    , length(length)
{}

size_t ArrayType::size() const
{
    return this->element_size * this->length;
}

size_t ArrayType::alignment() const
{
    return this->element_type->alignment();
}

void ArrayType::construct(char *address) const
{
    printf("constructing array from %p\n", address);
    for (size_t i = 0; i < this->length; ++i)
    {
        printf("constructing array element %zd: %p\n", i, address + this->element_size * i);
        this->element_type->construct(address + this->element_size * i);
    }
}

void ArrayType::destroy(char *address) const
{
    for (size_t i = 0; i < this->length; ++i)
    {
        this->element_type->destroy(address + this->element_size * (this->length - i - 1));
    }
}

Accessor ArrayType::operator[](size_t element_index) const
{
    size_t offset = this->at(element_index);
    return Accessor(this, this->element_type.get(), offset);
}

size_t ArrayType::at(size_t element_index) const
{
    if (element_index >= this->length)
    {
        throw IndexError("out of bounds index " + std::to_string(element_index));
    }
    return element_index * this->element_size;
}

Member::Member(std::shared_ptr<const Type> type, size_t offset)
    : type(std::move(type))
    , offset(offset)
{}

ObjectType::Builder::Builder(std::string name)
    : name(std::move(name))
{}

void ObjectType::Builder::member(const std::string& member_name, std::shared_ptr<const Type> member_type)
{
    size_t offset = align(this->members_size, member_type->alignment());
    this->members_size = offset + member_type->size();
    this->members.emplace_back(std::move(member_type), offset);
    this->members_lookup.emplace(member_name, this->members.size() - 1);
}

ObjectType::ObjectType(Builder&& builder)
    : members(std::move(builder.members))
    , members_lookup(std::move(builder.members_lookup))
    , members_size(builder.members_size)
{}

size_t ObjectType::size() const
{
    return this->members_size;
}

size_t ObjectType::alignment() const
{
    return this->members.empty() ? 0 : this->members.front().type->alignment();
}

void ObjectType::construct(char *address) const
{
    std::for_each(this->members.begin(), this->members.end(), [address](const Member& member)
    {
        member.type->construct(address + member.offset);
    });
}

void ObjectType::destroy(char *address) const
{
    std::for_each(this->members.rbegin(), this->members.rend(), [address](const Member& member)
    {
        member.type->destroy(address + member.offset);
    });
}

Accessor ObjectType::operator[](const std::string &member_name) const
{
    const Member& member = this->at(member_name);
    return Accessor(this, member.type.get(), member.offset);
}

const Member& ObjectType::at(const std::string& member_name) const
{
    MembersLookup::const_iterator find = this->members_lookup.find(member_name);
    if (find != this->members_lookup.end())
    {
        return this->members[find->second];
    }
    else
    {
        throw MemberError("no such member " + member_name);
    }
}

template<typename T>
Instance<T>::Instance(const T* type)
    : type(type)
{
    this->type->construct(this->address);
}

template<typename T>
Instance<T>::~Instance()
{
    this->type->destroy(this->address);
}

template<typename T>
Reference Instance<T>::operator[](const std::string& member_name)
{
    throw TypeError("member access is only available on objects!");
}

template<>
Reference Instance<ObjectType>::operator[](const std::string& member_name)
{
    const Member& member = this->type->at(member_name);
    return Reference(member.type.get(), this->address + member.offset);
}

template<typename T>
Reference Instance<T>::operator*()
{
    return Reference(this->type, this->address);
}

template<typename T>
Reference Instance<T>::operator[](size_t element_index)
{
    throw TypeError("indexing is only available on arrays!");
}

template<typename T>
template<typename U>
U* Instance<T>::operator[](const As<U>& as)
{
    return as(*this);
}

template<typename T>
template<typename U>
U& Instance<T>::operator[](const Is<U>& as)
{
    return as(*this);
}

template<>
Reference Instance<ArrayType>::operator[](size_t element_index)
{
    size_t offset = this->type->at(element_index);
    return Reference(this->type->element_type.get(), this->address + offset);
}

template<typename T>
template<typename U>
U* Instance<T>::as()
{
    return reinterpret_cast<U*>(this->address);
}

template<typename T>
template<typename U>
typename std::enable_if<std::is_base_of<T, ValueType>::value, U>::type& Instance<T>::is()
{
    if (this->type->info() == typeid(T))
    {
        return *reinterpret_cast<T*>(this->address);
    }
    else
    {
        throw TypeError("cast is only available for values!");
    }
}
