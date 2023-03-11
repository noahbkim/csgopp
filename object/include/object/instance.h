#pragma once

#include <memory>
#include <string_view>
#include "type.h"
#include "lens.h"

namespace object
{

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

    Reference operator[](auto argument) { return Reference(*this)[argument]; }
    ConstantReference operator[](auto argument) const { return ConstantReference(*this)[argument]; }
};

using Value = Instance<ValueType>;
using Array = Instance<ArrayType>;
using Object = Instance<ObjectType>;

}
