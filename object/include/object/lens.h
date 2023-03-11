#pragma once

#include <memory>
#include <string_view>
#include "type.h"
#include "magic.h"

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

    View operator[](const std::string& name) const;
    View operator[](size_t index) const;

    inline bool operator==(const View& v) const { return this->type == v.type && this->offset == v.offset; }
    inline bool operator!=(const View& v) const { return !(*this == v); }
    inline bool operator>(const View& v) const { return this->offset <= v.offset && v.bound() < this->bound(); }
    inline bool operator>=(const View& v) const { return *this == v || (*this) > v; }
    inline bool operator<(const View& v) const { return v.offset <= this->offset && this->bound() < v.bound(); }
    inline bool operator<=(const View& v) const { return *this == v || (*this) < v; }
};

struct Lens : public View
{
    std::shared_ptr<const Type> origin;

    Lens() = default;
    Lens(std::shared_ptr<const Type> origin)
        : View(origin)
        , origin(std::move(origin))
    {}
    Lens(std::shared_ptr<const Type> origin, View&& view)
        : View(std::move(view))
        , origin(origin)
    {}
    Lens(std::shared_ptr<const Type> origin, std::shared_ptr<const Type> type, size_t offset = 0)
        : View(std::move(type), offset)
        , origin(std::move(origin))
    {}

    Lens operator[](const std::string& name) const
    {
        return Lens(this->origin, std::move(View::operator[](name)));
    }

    Lens operator[](size_t index) const
    {
        return Lens(this->origin, std::move(View::operator[](index)));
    }

    inline bool operator==(const Lens& l) const { return this->origin == l.origin && View::operator==(l); }
    inline bool operator!=(const Lens& l) const { return this->origin == l.origin && View::operator!=(l); }
    inline bool operator>(const Lens& l) const { return this->origin == l.origin && View::operator>(l); }
    inline bool operator>=(const Lens& l) const { return this->origin == l.origin && View::operator>=(l); }
    inline bool operator<(const Lens& l) const { return this->origin == l.origin && View::operator<(l); }
    inline bool operator<=(const Lens& l) const { return this->origin == l.origin && View::operator<=(l); }
};

template<typename T>
struct Instance;

template<typename T>
struct BaseReference : public Lens
{
    std::shared_ptr<T> data;

    BaseReference() = default;
    BaseReference(std::shared_ptr<const Type> origin, std::shared_ptr<T> data)
        : Lens(std::move(origin))
        , data(std::move(data))
    {}
    BaseReference(std::shared_ptr<T> data, Lens&& lens)
        : Lens(std::move(lens))
        , data(std::move(data))
    {}
    BaseReference(
        std::shared_ptr<const Type> origin,
        std::shared_ptr<T> data,
        std::shared_ptr<const Type> type,
        size_t offset = 0
    )
        : Lens(std::move(origin), std::move(type), offset)
        , data(std::move(data))
    {}

    template<typename U>
    explicit BaseReference(const Instance<U>& instance)
        : Lens(instance.type)
        , data(instance.data)
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
    inline bool operator==(const BaseReference<U>& r) const { return this->data == r.data && View::operator==(r); }
    template<typename U>
    inline bool operator!=(const BaseReference<U>& r) const { return this->data == r.data && View::operator!=(r); }
    template<typename U>
    inline bool operator>(const BaseReference<U>& r) const { return this->data == r.data && View::operator>(r); }
    template<typename U>
    inline bool operator>=(const BaseReference<U>& r) const { return this->data == r.data && View::operator>=(r); }
    template<typename U>
    inline bool operator<(const BaseReference<U>& r) const { return this->data == r.data && View::operator<(r); }
    template<typename U>
    inline bool operator<=(const BaseReference<U>& r) const { return this->data == r.data && View::operator<=(r); }

    template<typename U>
    U& is() { return *reinterpret_cast<U*>(this->get()); }
    template<typename U>
    const U& is() const { return *reinterpret_cast<const U*>(this->get()); }

    template<typename U>
    U* as() { return reinterpret_cast<U*>(this->get()); }
    template<typename U>
    const U* as() const { return reinterpret_cast<const U*>(this->get()); }
};

using Reference = BaseReference<char[]>;
using ConstantReference = BaseReference<const char[]>;

}
