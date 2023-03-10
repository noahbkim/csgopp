#include "object/type.h"
#include "object/magic.h"
#include "object/error.h"

namespace object
{

template<typename T>
[[nodiscard]] size_t TrivialValueType<T>::size() const
{
    return sizeof(T);
}

template<typename T>
[[nodiscard]] size_t TrivialValueType<T>::alignment() const
{
    return std::alignment_of<T>();
}

template<typename T>
void TrivialValueType<T>::construct(char* address) const
{
    new(address) T;
}

template<typename T>
void TrivialValueType<T>::destroy(char* address) const
{
    reinterpret_cast<T*>(address)->~T();
}

template<typename T>
[[nodiscard]] const std::type_info& TrivialValueType<T>::info() const
{
    return typeid(T);
}

template<typename T>
[[nodiscard]] std::string TrivialValueType<T>::represent() const
{
    return typeid(T).name();
}

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
        // todo:optimization: cache
        this->element->construct(address + this->element->size() * i);
    }
}

void ArrayType::destroy(char* address) const
{
    for (size_t i = 0; i < this->length; ++i)
    {
        // todo:optimization: cache
        this->element->destroy(address + this->element->size() * (this->length - i - 1));
    }
}

ObjectType::Builder::Builder(auto&& base) : base(std::forward(base))
{
    // Copy everything over
    this->members = base->members;
    this->lookup = base->lookup;
    this->_size = base->size;
}

size_t ObjectType::Builder::embed(const ObjectType& type)
{
    if (type.members.empty())
    {
        // No alignment for zero-size type
        return this->_size;
    }

    // First, get the offset since it's computed for us; may be aligned
    const Member& front = type.members.front();
    size_t offset = this->member(front);

    // Rest
    for (size_t i = 1; i < type.members.size(); ++i)
    {
        const Member& member = type.members.at(i);
        this->member(member);
    }

    return offset;
}

size_t ObjectType::Builder::member(const Member& member)
{
    return this->member(member.name, member.type);
}

size_t ObjectType::Builder::member(std::string name, std::shared_ptr<const Type> type)
{
    // TODO: find a clean way to log if a member is hidden here
    size_t offset = align(this->_size, type->alignment());
    this->_size = offset + type->size();
    // Overwrite here; excludes always hide base class members, so we want to always point to child's member
    const Member& member = this->members.emplace_back(std::move(type), offset, std::move(name));
    this->lookup.emplace(std::string_view(member.name), this->members.size());
    return offset;
}

ObjectType::ObjectType(Builder&& builder)
    : name(std::move(builder.name))
    , lookup(std::move(builder.lookup))
    , members(std::move(builder.members))
    , base(std::move(builder.base))
    , _size(builder.size())
{
}

ObjectType::ObjectType(const Builder& builder)
    : name(builder.name)
    , lookup(builder.lookup)
    , members(builder.members)
    , base(builder.base)
    , _size(builder.size())
{
}

void ObjectType::construct(char* address) const
{
    for (const Member& member : this->members)
    {
        member.type->construct(address + member.offset);
    }
}

void ObjectType::destroy(char* address) const
{
    for (const Member& member : this->members)
    {
        member.type->construct(address + member.offset);
    }
}

[[nodiscard]] const ObjectType::Member& ObjectType::at(std::string_view name) const
{
    MemberLookup::const_iterator find = this->lookup.find(name);
    if (find != this->lookup.end())
    {
        // Can be left unchecked; we create the indices
        return this->members[find->second];
    }
    else
    {
        throw MemberError(concatenate("No such member ", name));
    }
}

}
