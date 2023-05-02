#pragma once

#include <string>
#include <memory>
#include "error.h"
#include "magic.h"
#include "type.h"
#include "type/value.h"

namespace objective
{

using objective::type::ValueType;

template<typename T, typename C>
T& is(const Type* type, C* data)
{
    if (type->info() != typeid(T))
    {
        throw TypeError(
            concatenate(
                "Value of type ",
                type->represent(),
                " cannot be accessed as ",
                typeid(T).name()
            )
        );
    }

    return *reinterpret_cast<T*>(data);
}

template<typename T, typename C>
T* as(const Type* type, C* data)
{
    return reinterpret_cast<T*>(data);
}

struct View
{
    size_t offset{0};
    std::shared_ptr<const Type> type;

    View() = default;
    explicit View(std::shared_ptr<const Type> type, size_t offset = 0)
        : type(std::move(type))
        , offset(offset)
    {}

    [[nodiscard]] size_t size() const { return this->type->size(); }
    [[nodiscard]] size_t bound() const { return this->offset + this->type->size(); }

    static View at(const Type* type, const std::string& name, size_t offset = 0);
    static View at(const Type* type, size_t index, size_t offset = 0);

    View operator[](const std::string& name) const { return at(this->type.get(), name, this->offset); }
    View operator[](size_t index) const { return at(this->type.get(), index, this->offset); }

    bool operator==(const View& v) const { return this->type == v.type && this->offset == v.offset; }
    bool operator!=(const View& v) const { return !(*this == v); }
    bool operator>(const View& v) const { return this->offset <= v.offset && v.bound() < this->bound(); }
    bool operator>=(const View& v) const { return *this == v || (*this) > v; }
    bool operator<(const View& v) const { return v.offset <= this->offset && this->bound() < v.bound(); }
    bool operator<=(const View& v) const { return *this == v || (*this) < v; }
};

}
