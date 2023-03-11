#include "object/error.h"
#include "object/lens.h"
#include "object/reference.h"
#include "object/instance.h"

namespace object
{

template<typename T>
static void validate(const Instance<T>* instance, const Type* origin)
{
    if (instance->type != origin)
    {
        throw TypeError(
            concatenate(
                "Cannot use lens with origin ",
                origin->represent(),
                " on instance of type ",
                instance->type->represent()
            )
        );
    }
}

template<typename T>
Reference Lens::operator()(std::shared_ptr<Instance<T>> instance)
{
    validate(instance.get(), this->type.get());
    return Reference(this->origin, instance->data, this->type, this->offset);
}

template<typename T>
ConstantReference Lens::operator()(std::shared_ptr<const Instance<T>> instance)
{
    validate(instance.get(), this->type.get());
    return ConstantReference(this->origin, instance->data, this->type, this->offset);
}

}
