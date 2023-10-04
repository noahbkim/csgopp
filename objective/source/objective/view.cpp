#include "objective/view.h"
#include "objective/type/object.h"
#include "objective/type/array.h"

namespace objective
{

using objective::type::ObjectType;
using objective::type::ArrayType;

View View::at(const Type* type, const std::string& name, size_t offset)
{
    auto* object_type = dynamic_cast<const ObjectType*>(type);
    if (object_type == nullptr)
    {
        throw TypeError("Can only access attributes on objects, not " + type->represent());
    }

    const ObjectType::Member& member = object_type->at(name);
    return View(member.type, offset + member.offset);
}

View View::at(const Type* type, size_t index, size_t offset)
{
    auto* array_type = dynamic_cast<const ArrayType*>(type);
    if (array_type == nullptr)
    {
        throw TypeError("Can only index arrays, not " + type->represent());
    }

    return View(array_type->element, offset + array_type->at(index));
}

}
