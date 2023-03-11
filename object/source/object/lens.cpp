#include "object/lens.h"
#include "object/error.h"

namespace object
{

View View::operator[](const std::string& name) const
{
    auto* object_type = dynamic_cast<const ObjectType*>(this->type.get());
    if (object_type != nullptr)
    {
        const ObjectType::Member& member = object_type->at(name);
        return View(member.type, this->offset + member.offset);
    }
    else
    {
        throw TypeError("Can only access attributes on objects, not " + this->type->represent());
    }
}

View View::operator[](size_t index) const
{
    auto* array_type = dynamic_cast<const ArrayType*>(this->type.get());
    if (array_type != nullptr)
    {
        return View(array_type->element, this->offset + array_type->at(index));
    }
    else
    {
        throw TypeError("Can only index arrays, not " + this->type->represent());
    }
}

}
