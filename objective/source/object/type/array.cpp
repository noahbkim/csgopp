#include "objective/error.h"
#include "objective/type/array.h"

namespace objective::type
{

ArrayType::ArrayType(std::shared_ptr<const Type> element, size_t length)
    : element(std::move(element))
    , length(length)
{
    this->_size = this->element->size() * length;
}

size_t ArrayType::size() const
{
    return this->_size;
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
    size_t element_size = this->element->size();
    for (size_t i = 0; i < this->length; ++i)
    {
        this->element->construct(address + element_size * i);
    }
}

void ArrayType::destroy(char* address) const
{
    size_t element_size = this->element->size();
    for (size_t i = 0; i < this->length; ++i)
    {
        this->element->destroy(address + element_size * (this->length - 1 - i));
    }
}

std::vector<std::string> ArrayType::keys() const
{
    throw TypeError("ArrayType has no keys!");
}

size_t ArrayType::count() const
{
    return this->length;
}

void ArrayType::emit(code::Declaration& declaration, code::Declaration::Member& member) const
{
    this->element->emit(declaration, member);
    member.array_sizes.push_back(this->length);
}

void ArrayType::emit(layout::Cursor& cursor) const
{
    size_t element_size = this->element->size();
    for (size_t i = 0; i < this->length; ++i)
    {
        size_t relative = element_size * i;
        cursor.write(std::to_string(i), relative);
        layout::Cursor indented(cursor.indent(relative));
        this->element->emit(cursor);
    }
}

size_t ArrayType::at(size_t index) const
{
    if (index >= this->length)
    {
        throw IndexError("Out of bounds index " + std::to_string(index));
    }
    return index * this->element->size();
}

}
