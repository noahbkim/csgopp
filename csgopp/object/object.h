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

namespace object
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

struct ConstReference;

struct Type;

template<typename T>
struct As
{
    std::shared_ptr<const Type> origin;
    size_t offset{0};

    As() = default;
    As(std::shared_ptr<const Type> origin, size_t offset);

    T* operator()(Reference& reference) const;
    const T* operator()(const Reference& reference) const;

    template<typename U>
    T* operator()(Instance<U>& instance) const;

    template<typename U>
    const T* operator()(const Instance<U>& instance) const;
};

template<typename T>
struct Is
{
    std::shared_ptr<const Type> origin;
    size_t offset{0};

    Is() = default;
    Is(std::shared_ptr<const Type> origin, size_t offset);

    T& operator()(Reference& reference) const;
    const T& operator()(const Reference& reference) const;

    template<typename U>
    T& operator()(Instance<const U>& instance) const;

    template<typename U>
    const T& operator()(const Instance<const U>& instance) const;
};

// How could this be abused?
struct Lens
{
    std::shared_ptr<const Type> type;
    size_t offset{0};

    Lens() = default;
    explicit Lens(std::shared_ptr<const Type> type, size_t offset = 0);

    [[nodiscard]] bool is_equal(const Lens& other) const;
    [[nodiscard]] bool is_subset_of(const Lens& other) const;
    [[nodiscard]] bool is_strict_subset_of(const Lens& other) const;
    [[nodiscard]] bool is_superset_of(const Lens& other) const;
    [[nodiscard]] bool is_strict_superset_of(const Lens& other) const;
};

struct Accessor : public Lens
{
    std::shared_ptr<const Type> origin;

    Accessor() = default;
    explicit Accessor(std::shared_ptr<const Type> type);
    explicit Accessor(std::shared_ptr<const Type> origin, std::shared_ptr<const Type> type, size_t offset);

    Accessor operator[](const std::string& member_name) const;
    Accessor operator[](size_t element_index) const;

    template<typename T>
    Reference operator()(Instance<const T>& instance);

    template<typename T>
    ConstReference operator()(const Instance<const T>& instance) const;

    template<typename T>
    As<T> as() const;

    template<typename T>
    Is<T> is() const;
};

struct Type
{
    virtual ~Type() = default;

    [[nodiscard]] virtual size_t size() const = 0;
    [[nodiscard]] virtual size_t alignment() const = 0;
    [[nodiscard]] virtual std::string represent() const = 0;

    virtual void construct(char* address) const = 0;
    virtual void destroy(char* address) const = 0;

    // TODO: these should just be NotImplemented if there's no reasonable default
    virtual void emit(code::Cursor<code::Declaration>& cursor) const = 0;
    virtual void emit(layout::Cursor& cursor) const = 0;
    virtual void format(const char* address, std::ostream& out) const = 0;
};

template<typename T>
void leak(T*)
{
}

template<typename T>
static std::shared_ptr<T> shared()
{
    static T type;
    static std::shared_ptr<T> pointer(&type, &leak<T>);
    return pointer;
}

struct Reference : public Lens
{
    std::shared_ptr<char[]> origin;

    Reference() = default;
    Reference(
        std::shared_ptr<char[]> origin,
        std::shared_ptr<const Type> type,
        size_t offset
    )
        : Lens(std::move(type), offset)
        , origin(std::move(origin))
    {
    }

    [[nodiscard]] char* address() const
    {
        return this->origin.get() + this->offset;
    }

    Reference operator[](const std::string& member_name) const;
    Reference operator[](size_t element_index) const;

    template<typename U>
    U* operator[](const As<U>& as);

    template<typename U>
    U& operator[](const Is<U>& is);

    template<typename U>
    U* as();

    template<typename U>
    U& is();
};

struct ConstReference : public Lens
{
    std::shared_ptr<const char[]> origin;

    ConstReference() = default;
    ConstReference(
        std::shared_ptr<const char[]> origin,
        std::shared_ptr<const Type> type,
        size_t offset
    )
        : Lens(std::move(type), offset)
        , origin(std::move(origin))
    {
    }

    [[nodiscard]] const char* address() const
    {
        return this->origin.get() + this->offset;
    }

    ConstReference operator[](const std::string& member_name) const;
    ConstReference operator[](size_t element_index) const;

    template<typename U>
    const U* operator[](const As<U>& as) const;

    template<typename U>
    const U& operator[](const Is<U>& is) const;

    template<typename U>
    const U* as() const;

    template<typename U>
    const U& is() const;
};

struct ValueType : public virtual Type
{
    [[nodiscard]] virtual const std::type_info& info() const = 0;

    [[nodiscard]] std::string represent() const override;

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
    [[nodiscard]] std::string represent() const override;

    void construct(char* address) const override;
    void destroy(char* address) const override;

    void emit(code::Cursor<code::Declaration>&) const override;
    void emit(layout::Cursor&) const override;

    [[nodiscard]] size_t at(size_t element_index) const;

    void format(const char* address, std::ostream& out) const override;
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
            code::Context<code::Declaration>* context = nullptr
        );

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
            code::Context<code::Declaration>* member_context = nullptr
        );
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
    [[nodiscard]] std::string represent() const override;

    void construct(char* address) const override;
    void destroy(char* address) const override;

    void emit(code::Cursor<code::Declaration>& cursor) const override;
    void emit(code::Cursor<code::Definition>& cursor) const;
    void emit(layout::Cursor& cursor) const override;

    [[nodiscard]] const Member& at(const std::string& member_name) const;

    [[nodiscard]] Members::const_iterator begin() const;
    [[nodiscard]] Members::const_iterator begin_self() const;
    [[nodiscard]] Members::const_iterator end() const;

    void format(const char* address, std::ostream& out) const override;
};

template<typename T>
struct Instance
{
    std::shared_ptr<const T> type;
    std::shared_ptr<char[]> address;

    explicit Instance(std::shared_ptr<const T> type) : type(std::move(type))
    {
        this->address = std::make_shared<char[]>(this->type->size());
        this->type->construct(this->address.get());
    }

    ~Instance()
    {
        this->type->destroy(this->address.get());
    }

    Reference operator[](const std::string& member_name);
    Reference operator[](size_t element_index);
    Reference operator*();

    ConstReference operator[](const std::string& member_name) const;
    ConstReference operator[](size_t element_index) const;
    ConstReference operator*() const;

    template<typename U>
    U* operator[](const As<U>& as);

    template<typename U>
    const U* operator[](const As<U>& as) const;

    template<typename U>
    U& operator[](const Is<U>& is);

    template<typename U>
    const U& operator[](const Is<U>& is) const;

    template<typename U>
    U* as();

    template<typename U>
    const U* as() const;

    template<typename U>
    U& is();

    template<typename U>
    const U& is() const;
};

using Object = Instance<ObjectType>;
using Array = Instance<ArrayType>;
using Value = Instance<ValueType>;

template<typename T>
std::ostream& operator<<(std::ostream& out, const Instance<T>* instance)
{
    instance->type->format(instance->address, out);
    return out;
}

template<typename T>
std::ostream& operator<<(std::ostream& out, const Instance<T>& instance)
{
    instance.type->format(instance.address, out);
    return out;
}

static inline void validate(const std::shared_ptr<const Type>& a, const std::shared_ptr<const Type>& b)
{
    if (a != b)
    {
        // TODO: maybe elaborate more on this error for Python users?
        throw TypeError("the lens and target types are incompatible!");
    }
}

template<typename T>
As<T>::As(std::shared_ptr<const Type> origin, size_t offset) : origin(std::move(origin)), offset(offset)
{
}

template<typename T>
T* As<T>::operator()(Reference& reference) const
{
    validate(this->origin, reference.type);
    return reinterpret_cast<T*>(reference.address() + this->offset);
}

template<typename T>
const T* As<T>::operator()(const Reference& reference) const
{
    validate(this->origin, reference.type);
    return reinterpret_cast<const T*>(reference.address() + this->offset);
}

template<typename T>
template<typename U>
T* As<T>::operator()(Instance<U>& instance) const
{
    validate(this->origin, instance.type);
    return reinterpret_cast<T*>(instance.address.get() + this->offset);
}

template<typename T>
template<typename U>
const T* As<T>::operator()(const Instance<U>& instance) const
{
    validate(this->origin, instance.type);
    return reinterpret_cast<const T*>(instance.address.get() + this->offset);
}

template<typename T>
Is<T>::Is(std::shared_ptr<const Type> origin, size_t offset) : origin(std::move(origin)), offset(offset)
{
}

template<typename T>
T& Is<T>::operator()(Reference& reference) const
{
    validate(this->origin, reference.type);
    return *reinterpret_cast<T*>(reference.address() + this->offset);
}

template<typename T>
const T& Is<T>::operator()(const Reference& reference) const
{
    validate(this->origin, reference.type);
    return *reinterpret_cast<T*>(reference.address() + this->offset);
}

template<typename T>
template<typename U>
T& Is<T>::operator()(Instance<const U>& instance) const
{
    validate(this->origin, instance.type);
    return *reinterpret_cast<T*>(instance.address.get() + this->offset);
}

template<typename T>
template<typename U>
const T& Is<T>::operator()(const Instance<const U>& instance) const
{
    validate(this->origin, instance.type);
    return *reinterpret_cast<const T*>(instance.address.get() + this->offset);
}

template<typename T>
Reference Accessor::operator()(Instance<const T>& instance)
{
    validate(this->origin, instance.type);
    return Reference(instance.address, this->type, instance.address.get() + this->offset);
}

template<typename T>
ConstReference Accessor::operator()(const Instance<const T>& instance) const
{
    validate(this->origin, instance.type);
    return ConstReference(instance.address, this->type, instance.address.get() + this->offset);
}

template<typename T>
As<T> Accessor::as() const
{
    return As<T>(this->origin, this->offset);
}

template<typename T>
Is<T> Accessor::is() const
{
    auto* value_type = dynamic_cast<const ValueType*>(this->type.get());
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

template<typename U>
U* Reference::operator[](const As<U>& as)
{
    return as(*this);
}

template<typename U>
U& Reference::operator[](const Is<U>& is)
{
    return is(*this);
}

template<typename U>
U* Reference::as()
{
    return reinterpret_cast<U*>(this->address());
}

template<typename U>
U& Reference::is()
{
    auto* value_type = dynamic_cast<const ValueType*>(this->type.get());
    if (value_type != nullptr)
    {
        if (value_type->info() == typeid(U))
        {
            return *reinterpret_cast<U*>(this->address());
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

template<typename U>
const U* ConstReference::operator[](const As<U>& as) const
{
    return as(*this);
}

template<typename U>
const U& ConstReference::operator[](const Is<U>& is) const
{
    return is(*this);
}

template<typename U>
const U* ConstReference::as() const
{
    return reinterpret_cast<const U*>(this->address());
}

template<typename U>
const U& ConstReference::is() const
{
    auto* value_type = dynamic_cast<const ValueType*>(this->type.get());
    if (value_type != nullptr)
    {
        if (value_type->info() == typeid(U))
        {
            return *reinterpret_cast<const U*>(this->address());
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
    return std::alignment_of<T>();
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
        new(address) T;
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
Reference Instance<T>::operator[](const std::string& member_name)
{
    auto* object_type = dynamic_cast<const ObjectType*>(this->type.get());
    if (object_type != nullptr)
    {
        const ObjectType::Member& member = object_type->at(member_name);
        return Reference(
            this->address,
            member.type,
            member.offset
        );
    }
    else
    {
        throw TypeError("member access is only available on objects!");
    }
}

template<typename T>
Reference Instance<T>::operator[](size_t element_index)
{
    auto* array_type = dynamic_cast<const ArrayType*>(this->type.get());
    if (array_type != nullptr)
    {
        size_t element_offset = array_type->at(element_index);
        return Reference(
            this->address,
            array_type->element_type,
            element_offset
        );
    }
    else
    {
        throw TypeError("indexing is only available on arrays!");
    }
}

template<typename T>
Reference Instance<T>::operator*()
{
    return Reference(this->type, this->address);
}

template<typename T>
ConstReference Instance<T>::operator[](const std::string& member_name) const
{
    auto* object_type = dynamic_cast<const ObjectType*>(this->type.get());
    if (object_type != nullptr)
    {
        const ObjectType::Member& member = object_type->at(member_name);
        return ConstReference(
            this->address,
            member.type,
            member.offset
        );
    }
    else
    {
        throw TypeError("member access is only available on objects!");
    }
}

template<typename T>
ConstReference Instance<T>::operator[](size_t element_index) const
{
    auto* array_type = dynamic_cast<const ArrayType*>(this->type.get());
    if (array_type != nullptr)
    {
        size_t element_offset = array_type->at(element_index);
        return ConstReference(
            this->address,
            array_type->element_type,
            element_offset
        );
    }
    else
    {
        throw TypeError("indexing is only available on arrays!");
    }
}

template<typename T>
ConstReference Instance<T>::operator*() const
{
    return ConstReference(this->address, this->type, this->address.get());
}

template<typename T>
template<typename U>
U* Instance<T>::operator[](const As<U>& as)
{
    return as(*this);
}

template<typename T>
template<typename U>
const U* Instance<T>::operator[](const As<U>& as) const
{
    return as(*this);
}

template<typename T>
template<typename U>
U& Instance<T>::operator[](const Is<U>& is)
{
    return is(*this);
}

template<typename T>
template<typename U>
const U& Instance<T>::operator[](const Is<U>& is) const
{
    return is(*this);
}

template<typename T>
template<typename U>
U* Instance<T>::as()
{
    return reinterpret_cast<U*>(this->address.get());
}

template<typename T>
template<typename U>
const U* Instance<T>::as() const
{
    return reinterpret_cast<const U*>(this->address.get());
}

template<typename T>
template<typename U>
U& Instance<T>::is()
{
    auto* value_type = dynamic_cast<const ValueType*>(this->type.get());
    if (value_type != nullptr)
    {
        if (value_type->info() == typeid(U))
        {
            return *reinterpret_cast<U*>(this->address.get());
        }
        else
        {
            throw TypeError("accessor target does not match its origin");
        }
    }
    else
    {
        throw TypeError("cast is only available for values!");
    }
}

template<typename T>
template<typename U>
const U& Instance<T>::is() const
{
    auto* value_type = dynamic_cast<const ValueType*>(this->type.get());
    if (value_type != nullptr)
    {
        if (value_type->info() == typeid(U))
        {
            return *reinterpret_cast<U*>(this->address.get());
        }
        else
        {
            throw TypeError("accessor target does not match its origin");
        }
    }
    else
    {
        throw TypeError("cast is only available for values!");
    }
}

}
