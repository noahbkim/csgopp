#pragma once

#include <memory>
#include "lens.h"

namespace object
{

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
    BaseReference(
        std::shared_ptr<const Type> origin,
        std::shared_ptr<T> data,
        std::shared_ptr<const Type> type,
        size_t offset = 0
    )
        : Lens(std::move(origin), std::move(type), offset)
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