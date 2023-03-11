#pragma once

#include <memory>
#include <string_view>
#include "type.h"
#include "lens.h"

namespace object
{

struct Metaclass
{
    std::shared_ptr<const Type> type;

    inline Lens view() const { return Lens(this->type); }

    inline Lens operator[](const std::string& name) const
    {
        return Lens(this->type, std::move(at(this->type.get(), name)));
    }

    inline Lens operator[](const size_t index) const
    {
        return Lens(this->type, std::move(at(this->type.get(), index)));
    }
};

template<typename T>
struct Instance
{
    std::shared_ptr<const T> type;
    std::shared_ptr<char[]> data;

    Instance() = default;
    explicit Instance(std::shared_ptr<const T> type)
        : type(std::move(type))
    {
        this->data = std::make_shared<char[]>(this->type->size());
        this->type->construct(this->data.get());
    }
    Instance(std::shared_ptr<const T> type, std::shared_ptr<char[]> data)
        : type(std::move(type))
        , data(std::move(data))
    {
        this->type->construct(this->data.get());
    }

    virtual ~Instance()
    {
        this->type->destroy(this->data.get());
    }

    inline Reference view() { return Reference(this->type, this->data); }

    Reference operator[](const std::string& name)
    {
        return Reference(this->type, this->data, std::move(at(this->type.get(), name)));
    }

    Reference operator[](size_t index)
    {
        return Reference(this->type, this->data, std::move(at(this->type.get(), index)));
    }

    ConstantReference operator[](const std::string& name) const
    {
        return ConstantReference(this->type, this->data, std::move(at(this->type.get(), name)));
    }

    ConstantReference operator[](size_t index) const
    {
        return ConstantReference(this->type, this->data, std::move(at(this->type.get(), index)));
    }

    template<typename U>
    U& is() { return object::is<U>(this->type.get(), this->data.get()); }
    template<typename U>
    const U& is() const { object::is<U>(this->type.get(), this->data.get()); }

    template<typename U>
    U* as() { return object::as<U>(this->type.get(), this->data.get()); }
    template<typename U>
    const U* as() const { return object::as<U>(this->type.get(), this->data.get()); }
};

using Value = Instance<ValueType>;
using Array = Instance<ArrayType>;
using Object = Instance<ObjectType>;

}
