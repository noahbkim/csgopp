#pragma once

#include <memory>
#include "type.h"
#include "lens.h"
#include "magic.h"

namespace object
{

template<typename T>
struct Handle
{
    std::shared_ptr<T> self;

    Handle() = default;
    Handle(auto& self) : self(self) {}
    Handle(auto&& self) : self(std::move(self)) {}

    // Forward pointer accessors, is this unintuitive?
    std::shared_ptr<T>& operator*() { return this->self; }
    const std::shared_ptr<T>& operator*() const { return this->self; }
    T* operator->() { return this->self.operator->(); }
    const T* operator->() const { return this->self.operator->(); }

    // Allow conversion into shared_ptr
    operator std::shared_ptr<T>() const { return this->self; }
    operator std::shared_ptr<T>&() { return this->self; }
    operator const std::shared_ptr<T>&() const { return this->self; }

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
