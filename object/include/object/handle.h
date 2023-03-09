#pragma once

#include <memory>
#include "type.h"
#include "magic.h"

namespace object
{

template<typename T>
struct Handle
{
    std::shared_ptr<T> self;

    // Forward pointer accessors
    T& operator*() { return this->self->operator*(); }
    const T& operator*() const { return this->self->operator*(); }
    T* operator->() { return this->self->operator->(); }
    const T* operator->() const { return this->self->operator->(); }

    // Type handle
    auto operator[](auto argument) const
    {
        if constexpr (std::is_base_of<Type, T>::value)
        {
            return Accessor(this->self)[argument];
        }
        else if constexpr (std::is_base_of<Instance, T>::value)
        {
            if constexpr (std::is_const<T>::value)
            {
                return ConstantReference(this->self)[argument];
            }
            else
            {
                return Reference(this->self)[argument];
            }
        }
        else
        {
            static_assert(dependent_false<T>::value);
        }
    }
};

}
