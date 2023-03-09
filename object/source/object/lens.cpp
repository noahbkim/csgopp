#include "object/lens.h"
#include "object/exception.h"

namespace object
{

template<typename T>
Lens<T> Lens<T>::operator[](std::string_view name) const
{
    auto* object_type = dynamic_cast<const ObjectType*>(this->type.get());
    if (object_type != nullptr)
    {
        const ObjectType::Member& member = object_type->at(name);
        return Accessor(this->origin, member.type, this->offset + member.offset);
    }
    else
    {
        throw TypeError("Can only access attributes on objects, not " + this->type->represent());
    }
}

template<typename T>
Lens<T> Lens<T>::operator[](size_t index) const
{
    auto* array_type = dynamic_cast<const ArrayType*>(this->type.get());
    if (array_type != nullptr)
    {
        return Lens(this->origin, array_type->element, array_type->at(index));
    }
    else
    {
        throw TypeError("Can only index arrays, not " + this->type->represent());
    }
}

}
