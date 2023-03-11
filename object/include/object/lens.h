#pragma once

#include <memory>
#include <utility>
#include <string_view>
#include "type.h"
#include "magic.h"
#include "error.h"

namespace object
{

struct View;
struct Lens;

View at(const Type* type, const std::string& name, size_t offset = 0);
View at(const Type* type, size_t index, size_t offset = 0);

template<typename T>
T& is(const Type* type, char* data)
{
    auto* value_type = dynamic_cast<const ValueType*>(type);
    if (value_type == nullptr)
    {
        throw TypeError("Can only cast value types, not " + type->represent());
    }
    if (value_type->info() != typeid(T))
    {
        throw TypeError(
            concatenate(
                "Value of type ",
                value_type->info().name(),
                " cannot be accessed as ",
                typeid(T).name()
            )
        );
    }

    return *reinterpret_cast<T*>(data);
}

template<typename T>
T* as(const Type* type, char* data)
{
    return reinterpret_cast<T*>(data);
}

struct View
{
    size_t offset{0};
    std::shared_ptr<const Type> type;

    View() = default;
    View(std::shared_ptr<const Type> type, size_t offset = 0) : type(std::move(type)), offset(offset) {}

    [[nodiscard]] size_t size() const { return this->type->size(); }
    [[nodiscard]] size_t bound() const { return this->offset + this->type->size(); }

    View operator[](const std::string& name) const { return at(this->type.get(), name, this->offset); }
    View operator[](size_t index) const { return at(this->type.get(), index, this->offset); }

    bool operator==(const View& v) const { return this->type == v.type && this->offset == v.offset; }
    bool operator!=(const View& v) const { return !(*this == v); }
    bool operator>(const View& v) const { return this->offset <= v.offset && v.bound() < this->bound(); }
    bool operator>=(const View& v) const { return *this == v || (*this) > v; }
    bool operator<(const View& v) const { return v.offset <= this->offset && this->bound() < v.bound(); }
    bool operator<=(const View& v) const { return *this == v || (*this) < v; }
};

struct Lens : public View
{
    std::shared_ptr<const Type> origin;

    Lens() = default;
    Lens(std::shared_ptr<const Type> origin) : View(origin), origin(std::move(origin)) {}
    Lens(std::shared_ptr<const Type> origin, View&& view) : View(std::move(view)), origin(origin) {}
    Lens(std::shared_ptr<const Type> origin, std::shared_ptr<const Type> type, size_t offset = 0)
        : View(std::move(type), offset)
        , origin(std::move(origin))
    {}

    Lens operator[](const std::string& name) const { return Lens(this->origin, std::move(View::operator[](name))); }
    Lens operator[](size_t index) const { return Lens(this->origin, std::move(View::operator[](index))); }

    bool operator==(const Lens& l) const { return this->origin == l.origin && View::operator==(l); }
    bool operator!=(const Lens& l) const { return this->origin == l.origin && View::operator!=(l); }
    bool operator>(const Lens& l) const { return this->origin == l.origin && View::operator>(l); }
    bool operator>=(const Lens& l) const { return this->origin == l.origin && View::operator>=(l); }
    bool operator<(const Lens& l) const { return this->origin == l.origin && View::operator<(l); }
    bool operator<=(const Lens& l) const { return this->origin == l.origin && View::operator<=(l); }
};

template<typename T>
struct BaseReference : public Lens
{
    std::shared_ptr<T> data;

    BaseReference() = default;
    BaseReference(std::shared_ptr<T> data, Lens&& lens)
        : Lens(std::move(lens))
        , data(std::move(data))
    {}
    BaseReference(std::shared_ptr<const Type> origin, std::shared_ptr<T> data)
        : Lens(std::move(origin))
        , data(std::move(data))
    {}
    BaseReference(std::shared_ptr<const Type> origin, std::shared_ptr<T> data, View&& view)
        : Lens(std::move(origin), std::move(view))
        , data(std::move(data))
    {}

    [[nodiscard]] char* get() { return this->data.get() + this->offset; }
    [[nodiscard]] const char* get() const { return this->data.get() + this->offset; }

    BaseReference operator[](const std::string& name) const
    {
        return BaseReference(this->data, std::move(Lens::operator[](name)));
    }

    BaseReference operator[](size_t index) const
    {
        return BaseReference(this->data, std::move(Lens::operator[](index)));
    }

    template<typename U>
    bool operator==(const BaseReference<U>& r) const { return this->data == r.data && View::operator==(r); }
    template<typename U>
    bool operator!=(const BaseReference<U>& r) const { return this->data == r.data && View::operator!=(r); }
    template<typename U>
    bool operator>(const BaseReference<U>& r) const { return this->data == r.data && View::operator>(r); }
    template<typename U>
    bool operator>=(const BaseReference<U>& r) const { return this->data == r.data && View::operator>=(r); }
    template<typename U>
    bool operator<(const BaseReference<U>& r) const { return this->data == r.data && View::operator<(r); }
    template<typename U>
    bool operator<=(const BaseReference<U>& r) const { return this->data == r.data && View::operator<=(r); }

    template<typename U>
    U& is() { return object::is<U>(this->type.get(), this->get()); }
    template<typename U>
    const U& is() const { object::is<U>(this->type.get(), this->get()); }

    template<typename U>
    U* as() { return object::as<U>(this->type.get(), this->get()); }
    template<typename U>
    const U* as() const { return object::as<U>(this->type.get(), this->get()); }
};

using Reference = BaseReference<char[]>;
using ConstantReference = BaseReference<const char[]>;

}
