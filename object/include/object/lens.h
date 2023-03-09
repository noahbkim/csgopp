#pragma once

#include <memory>
#include <string_view>
#include "type.h"
#include "instance.h"

namespace object
{

struct View
{
    size_t offset{0};
    std::shared_ptr<const Type> type;

    View() = default;
    View(std::shared_ptr<const Type> type, size_t offset = 0)
        : type(std::move(type))
        , offset(offset)
    {}

    [[nodiscard]] size_t size() const { return this->type->size(); }
    [[nodiscard]] size_t bound() const { return this->offset + this->type->size(); }

    bool operator==(const View& other) const { return this->type == other.type && this->offset == other.offset; }
    bool operator!=(const View& other) const { return !(*this == other); }
    bool operator>(const View& other) const { return this->offset <= other.offset && other.bound() < this->bound(); }
    bool operator>=(const View& other) const { return *this == other || (*this) > other; }
    bool operator<(const View& other) const { return other.offset <= this->offset && this->bound() < other.bound(); }
    bool operator<=(const View& other) const { return *this == other || (*this) < other; }
};

namespace lens
{

template<typename T>
inline bool compatible(const T* t, const T* u) { return t == u; }

template<typename T, typename U>
inline bool compatible(const T* t, const U* u) { static_assert(dependent_false<T>::value); }

template<>
inline bool compatible<Type, Instance>(const Type* t, const Instance* u) { return t == u->type().get(); }

template<>
inline bool compatible<Instance, Type>(const Instance* t, const Type* u) { return compatible(u, t); }

}

template<typename T>
struct Lens : public View
{
    std::shared_ptr<T> origin;

    Lens() = default;
    Lens(std::shared_ptr<T> origin, std::shared_ptr<const Type> type, size_t offset)
        : View(std::move(type), offset)
        , origin(std::move(origin))
    {}

    Lens operator[](std::string_view name) const;
    Lens operator[](size_t index) const;

    template<typename U>
    bool compatible(const Lens<U>& other) const { return lens::compatible(this->origin.get(), other.origin.get()); }

    template<typename U>
    bool operator==(const Lens<U>& other) const { return this->compatible(other) && View::operator==(other); }
    template<typename U>
    bool operator!=(const Lens<U>& other) const { return this->compatible(other) && View::operator!=(other); }
    template<typename U>
    bool operator>(const Lens<U>& other) const { return this->compatible(other) && View::operator>(other); }
    template<typename U>
    bool operator>=(const Lens<U>& other) const { return this->compatible(other) && View::operator>=(other); }
    template<typename U>
    bool operator<(const Lens<U>& other) const { return this->compatible(other) && View::operator<(other); }
    template<typename U>
    bool operator<=(const Lens<U>& other) const { return this->compatible(other) && View::operator<=(other); }
};

struct Accessor : public Lens<const Type>
{
    using Lens::Lens;
    explicit Accessor(const std::shared_ptr<const Type>& origin) : Lens(origin, origin, 0) {}
};

struct Reference : public Lens<Instance>
{
    using Lens::Lens;
    explicit Reference(const std::shared_ptr<Instance>& origin) : Lens(origin, origin->type(), 0) {}
};

struct ConstantReference : public Lens<const Instance>
{
    using Lens::Lens;
    explicit ConstantReference(const std::shared_ptr<const Instance>& origin) : Lens(origin, origin->type(), 0) {}
};

}
