#include "object/type.h"

namespace object
{

ArrayType::ArrayType(std::shared_ptr<const Type> element, size_t length)
    : element(element)
    , length(length)
{
}

size_t ArrayType::size() const
{
    // todo:optimization: cache
    return this->element->size() * this->length;
}

size_t ArrayType::alignment() const
{
    return this->element->alignment();
}

std::string ArrayType::represent() const
{
    return this->element->represent() + "[" + std::to_string(this->length) + "]";
}

void ArrayType::construct(char* address) const
{
    for (size_t i = 0; i < this->length; ++i)
    {
        this->element->construct(address + this->element->size() * i);
    }
}

void ArrayType::destroy(char* address) const
{
    for (size_t i = 0; i < this->length; ++i)
    {
        this->element->destroy(address + this->element->size() * (this->length - i - 1));
    }
}

}
