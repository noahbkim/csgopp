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

struct Instance;

struct Reference;

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
    T* operator()(Instance& instance) const;
    const T* operator()(const Instance& instance) const;
};

template<typename T>
struct Is
{
    std::shared_ptr<const Type> origin;
    size_t offset{0};

    Is() = default;
    Is(std::shared_ptr<const Type> origin, size_t offset);
    Is(std::shared_ptr<const Type>&& origin, size_t offset);

    T& operator()(Reference& reference) const;
    const T& operator()(const Reference& reference) const;
    T& operator()(Instance& instance) const;
    const T& operator()(const Instance& instance) const;
};

struct Accessor
{
    std::shared_ptr<const struct Type> origin;
    std::shared_ptr<const struct Type> type;
    size_t offset{0};

    Accessor() = default;
    Accessor(std::shared_ptr<const struct Type> origin, std::shared_ptr<const struct Type> type, size_t offset);

    Accessor operator[](const std::string& member_name) const;
    Accessor operator[](size_t element_index) const;
    Reference operator()(Instance& instance) const;

    template<typename T>
    As<T> as() const;

    template<typename T>
    Is<T> is() const;

    [[nodiscard]] bool contains(const Accessor& other) const;
    bool operator==(const Accessor& other) const;
    bool operator>(const Accessor& other) const;
    bool operator>=(const Accessor& other) const;
};

struct Type
{
    std::weak_ptr<const Type> self;

    virtual ~Type() = default;

    [[nodiscard]] virtual size_t size() const = 0;
    [[nodiscard]] virtual size_t alignment() const = 0;

    virtual void construct(char* address) const = 0;
    virtual void destroy(char* address) const = 0;

    [[nodiscard]] virtual Accessor operator[](const std::string& member_name) const;
    [[nodiscard]] virtual Accessor operator[](size_t element_index) const;

    virtual void emit(code::Cursor<code::Declaration>& cursor) const = 0;
    virtual void emit(layout::Cursor& cursor) const = 0;
    virtual void represent(const char* address, std::ostream& out) const = 0;

    template<typename I = Instance, typename... Args>
    [[nodiscard]] std::shared_ptr<Instance> instantiate(Args&&... args) const
    {
        size_t buffer_size = sizeof(I) + this->size();
        std::shared_ptr<uint8_t[]> buffer = std::make_shared<uint8_t[]>(buffer_size);
        std::shared_ptr<Instance> instance = std::reinterpret_pointer_cast<Instance>(std::move(buffer));
        new(instance.get()) I(std::move(this->self.lock()), std::move(args)...);
        instance->self = instance;
        return instance;
    }
};

template<typename T>
struct instantiate
{
    template<typename... Args>
    static std::shared_ptr<T> anonymous(Args&&... args)
    {
        auto type = std::make_shared<T>(std::move(args)...);
        type->self = type;
        return type;
    }

    static std::shared_ptr<T> shared()
    {
        static T type;
        static std::shared_ptr<T> pointer(&type, &leak<T>);
        type.self = pointer;  // This seems potentially annoying
        return pointer;
    }
};

struct Reference
{
    std::shared_ptr<struct Instance> origin;
    std::shared_ptr<const Type> type;
    char* address{nullptr};

    Reference(
        std::shared_ptr<struct Instance> origin,
        std::shared_ptr<const Type> type,
        char* address
    );

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

struct ConstReference
{
    std::shared_ptr<const struct Instance> origin;
    std::shared_ptr<const Type> type;
    const char* address{nullptr};

    ConstReference(
        std::shared_ptr<const struct Instance> origin,
        std::shared_ptr<const Type> type,
        const char* address
    );

    ConstReference operator[](const std::string& member_name) const;
    ConstReference operator[](size_t element_index) const;

    template<typename T>
    const T* operator[](const As<T>& as) const;

    template<typename T>
    const T& operator[](const Is<T>& is) const;

    template<typename T>
    const T* as() const;

    template<typename T>
    const T& is() const;
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

    void represent(const char* address, std::ostream& out) const override;
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

    void represent(const char* address, std::ostream& out) const override;
};

struct Instance
{
    std::shared_ptr<const Type> type;
    std::weak_ptr<struct Instance> self;
    char address[];

    ~Instance()
    {
        this->type->destroy(this->address);
    }

    Instance(Instance& other) = delete;
    Instance(Instance&& other) = delete;
    Instance& operator=(Instance& other) = delete;
    Instance& operator=(Instance&& other) = delete;

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
    U& operator[](const Is<U>& as);

    template<typename U>
    const U& operator[](const Is<U>& as) const;

    template<typename T>
    T* as();

    template<typename T>
    const T* as() const;

    template<typename T>
    T& is();

    template<typename T>
    const T& is() const;

protected:
    friend Type;

    explicit Instance(std::shared_ptr<const Type>&& type) : type(std::move(type))
    {
        this->type->construct(this->address);
    }
};

std::ostream& operator<<(std::ostream& out, const Instance* instance);
std::ostream& operator<<(std::ostream& out, const Instance& instance);
std::ostream& operator<<(std::ostream& out, const Reference& instance);
std::ostream& operator<<(std::ostream& out, const ConstReference& instance);

template<typename T>
As<T>::As(std::shared_ptr<const Type> origin, size_t offset)
    : origin(std::move(origin))
    , offset(offset)
{
}

template<typename T>
T* As<T>::operator()(Reference& reference) const
{
    if (reference.type == this->origin)
    {
        return reinterpret_cast<T*>(reference.address);
    }
    else
    {
        throw TypeError("accessor target does not match its origin");
    }
}

template<typename T>
const T* As<T>::operator()(const Reference& reference) const
{
    if (reference.type == this->origin)
    {
        return reinterpret_cast<const T*>(reference.address);
    }
    else
    {
        throw TypeError("accessor target does not match its origin");
    }
}

template<typename T>
T* As<T>::operator()(Instance& instance) const
{
    if (instance.type == this->origin)
    {
        return reinterpret_cast<T*>(instance.address);
    }
    else
    {
        throw TypeError("accessor target does not match its origin");
    }
}

template<typename T>
const T* As<T>::operator()(const Instance& instance) const
{
    if (instance.type == this->origin)
    {
        return reinterpret_cast<const T*>(instance.address);
    }
    else
    {
        throw TypeError("accessor target does not match its origin");
    }
}

template<typename T>
Is<T>::Is(std::shared_ptr<const Type> origin, size_t offset)
    : origin(std::move(origin))
    , offset(offset)
{
}

template<typename T>
Is<T>::Is(std::shared_ptr<const Type>&& origin, size_t offset)
    : origin(std::move(origin))
    , offset(offset)
{
}

template<typename T>
T& Is<T>::operator()(Reference& reference) const
{
    if (reference.type == this->origin)
    {
        return *reinterpret_cast<T*>(reference.address + this->offset);
    }
    else
    {
        throw TypeError("accessor target does not match its origin");
    }
}

template<typename T>
const T& Is<T>::operator()(const Reference& reference) const
{
    if (reference.type == this->origin)
    {
        return *reinterpret_cast<T*>(reference.address + this->offset);
    }
    else
    {
        throw TypeError("accessor target does not match its origin");
    }
}

template<typename T>
T& Is<T>::operator()(Instance& instance) const
{
    if (instance.type == this->origin)
    {
        return *reinterpret_cast<T*>(instance.address + this->offset);
    }
    else
    {
        throw TypeError("accessor target does not match its origin");
    }
}

template<typename T>
const T& Is<T>::operator()(const Instance& instance) const
{
    if (instance.type == this->origin)
    {
        return *reinterpret_cast<const T*>(instance.address + this->offset);
    }
    else
    {
        throw TypeError("accessor target does not match its origin");
    }
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

template<typename T>
void leak(T*)
{
}

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
    auto* value_type = dynamic_cast<const ValueType*>(this->type.get());
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
const T* ConstReference::operator[](const As<T>& as) const
{
    return as(*this);
}

template<typename T>
const T& ConstReference::operator[](const Is<T>& is) const
{
    return is(*this);
}

template<typename T>
const T* ConstReference::as() const
{
    return reinterpret_cast<const T*>(this->address);
}

template<typename T>
const T& ConstReference::is() const
{
    auto* value_type = dynamic_cast<const ValueType*>(this->type.get());
    if (value_type != nullptr)
    {
        if (value_type->info() == typeid(T))
        {
            return *reinterpret_cast<const T*>(this->address);
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

template<typename U>
U* Instance::operator[](const As<U>& as)
{
    return as(*this);
}

template<typename U>
const U* Instance::operator[](const As<U>& as) const
{
    return as(*this);
}

template<typename U>
U& Instance::operator[](const Is<U>& as)
{
    return as(*this);
}

template<typename U>
const U& Instance::operator[](const Is<U>& as) const
{
    return as(*this);
}

template<typename T>
T* Instance::as()
{
    return reinterpret_cast<T*>(this->address);
}

template<typename T>
const T* Instance::as() const
{
    return reinterpret_cast<const T*>(this->address);
}

template<typename T>
T& Instance::is()
{
    auto* value_type = dynamic_cast<const ValueType*>(this->type.get());
    if (value_type != nullptr)
    {
        if (value_type->info() == typeid(T))
        {
            return *reinterpret_cast<T*>(this->address);
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
const T& Instance::is() const
{
    auto* value_type = dynamic_cast<const ValueType*>(this->type.get());
    if (value_type != nullptr)
    {
        if (value_type->info() == typeid(T))
        {
            return *reinterpret_cast<T*>(this->address);
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
