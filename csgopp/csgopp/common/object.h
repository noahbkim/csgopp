#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <typeinfo>
#include <algorithm>
#include <stdexcept>
#include <ostream>
#include <absl/container/flat_hash_map.h>

#include "code.h"
#include "layout.h"

namespace csgopp::common::object
{

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
        return offset;
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

    virtual void emit(code::Cursor<code::Declaration>&) const = 0;
    virtual void emit(layout::Cursor&) const = 0;
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
    T& operator[](const Is<T>& is);

    template<typename T>
    T* as();

    template<typename T>
    T& is();
};

struct ValueType : public virtual Type
{
    [[nodiscard]] virtual const std::type_info& info() const = 0;
    void emit(code::Cursor<code::Declaration>&) const override;
    void emit(layout::Cursor&) const override;
};

template<typename T>
struct DefaultValueType : public ValueType
{
    using Value = T;

    [[nodiscard]] size_t size() const override;
    [[nodiscard]] size_t alignment() const override;
    [[nodiscard]] const std::type_info& info() const override;

    void construct(char* address) const override;
    void destroy(char* address) const override;
};

struct ArrayType : public virtual Type
{
    std::shared_ptr<const Type> element_type;
    size_t element_size;
    size_t length;

    ArrayType(std::shared_ptr<const Type> element_type, size_t length);
    [[nodiscard]] size_t size() const override;
    [[nodiscard]] size_t alignment() const override;

    void construct(char* address) const override;
    void destroy(char* address) const override;

    void emit(code::Cursor<code::Declaration>&) const override;
    void emit(layout::Cursor&) const override;

    [[nodiscard]] Accessor operator[](size_t element_index) const override;

    [[nodiscard]] size_t at(size_t element_index) const;
};

struct ObjectType : public virtual Type
{
    struct Member
    {
        std::shared_ptr<const Type> type;
        size_t offset;
        std::string name;
        code::Context<code::Declaration>* context{nullptr};

        Member(
            std::shared_ptr<const Type> type,
            size_t offset,
            std::string&& name,
            code::Context<code::Declaration>* context = nullptr);

        void emit(code::Cursor<code::Declaration>) const;
    };

    using Members = std::vector<Member>;
    using MembersLookup = absl::flat_hash_map<std::string, size_t>;

    struct Builder
    {
        Members members;
        MembersLookup members_lookup;
        size_t members_size{0};
        std::string name;
        const ObjectType* base{nullptr};
        code::Context<code::Definition>* context{nullptr};

        Builder() = default;
        explicit Builder(const ObjectType* base_object_type);

        size_t embed(const ObjectType* object_type);
        size_t member(const Member& member);
        size_t member(
            std::string member_name,
            std::shared_ptr<const Type> member_type,
            code::Context<code::Declaration>* member_context = nullptr);
    };

    Members members;
    MembersLookup members_lookup;
    size_t members_size;
    std::string name;
    const ObjectType* base;
    code::Context<code::Definition>* context{nullptr};

    explicit ObjectType(Builder&& builder);
    [[nodiscard]] size_t size() const override;
    [[nodiscard]] size_t alignment() const override;

    void construct(char* address) const override;
    void destroy(char* address) const override;

    void emit(code::Cursor<code::Declaration>& cursor) const override;
    void emit(code::Cursor<code::Definition>& cursor) const;
    void emit(layout::Cursor& cursor) const override;

    [[nodiscard]] Accessor operator[](const std::string& member_name) const override;
    [[nodiscard]] const Member& at(const std::string& member_name) const;

    [[nodiscard]] Members::const_iterator begin() const;
    [[nodiscard]] Members::const_iterator begin_self() const;
    [[nodiscard]] Members::const_iterator end() const;
};

template<typename T>
struct Instance
{
    const T* type;
    char* address;

    Instance(const T* type, char* address);
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
    template<typename T, typename I = Instance<T>, typename... Args>
    I* instantiate(const T* type);
};

using Object = Instance<ObjectType>;
using Array = Instance<ArrayType>;

template<typename T, typename I = Instance<T>, typename... Args>
I* instantiate(const T* type, Args&&... args)
{
    char* address = new char[sizeof(I) + type->size()];
    return new (address) I(type, address + sizeof(I), args...);
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

template<typename T>
void leak(T*) {}

template<typename T>
std::shared_ptr<T> shared()
{
    static T type;
    static std::shared_ptr<T> pointer(&type, &leak<T>);
    return pointer;
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

template<typename T>
Instance<T>::Instance(const T* type, char* address)
    : type(type)
    , address(address)
{
    this->type->construct(this->address);
}

template<typename T>
Instance<T>::~Instance()
{
    this->type->destroy(this->address);
}

template<typename T>
Reference Instance<T>::operator[](const std::string&)
{
    throw TypeError("member access is only available on objects!");
}

template<>
inline Reference Instance<ObjectType>::operator[](const std::string& member_name)
{
    const ObjectType::Member& member = this->type->at(member_name);
    return Reference(member.type.get(), this->address + member.offset);
}

template<typename T>
Reference Instance<T>::operator[](size_t)
{
    throw TypeError("indexing is only available on arrays!");
}

template<>
inline Reference Instance<ArrayType>::operator[](size_t element_index)
{
    size_t offset = this->type->at(element_index);
    return Reference(this->type->element_type.get(), this->address + offset);
}

template<typename T>
Reference Instance<T>::operator*()
{
    return Reference(this->type, this->address);
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

}
