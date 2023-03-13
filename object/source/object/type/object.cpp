#include "object/error.h"
#include "object/magic.h"
#include "object/type/object.h"

namespace object::type
{

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

size_t ObjectType::Builder::member(const std::string& name, std::shared_ptr<const Type> type)
{
    // TODO: find a clean way to log if a member is hidden here
    size_t offset = align(this->_size, type->alignment());
    this->_size = offset + type->size();
    // Overwrite here; excludes always hide base class members, so we want to always point to child's member
    this->lookup[name] = this->members.size();
    const Member& member = this->members.emplace_back(name, std::move(type), offset);
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

[[nodiscard]] const ObjectType::Member& ObjectType::at(const std::string& name) const
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