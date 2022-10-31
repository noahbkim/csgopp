#include "object.h"

namespace csgopp::common::object
{


Reference Accessor::bind(char* address) const
{
    return Reference(this->type, address + this->offset);
}

[[nodiscard]] Accessor Type::operator[](const std::string& member_name) const
{
    throw TypeError("member access is only available on objects!");
}

[[nodiscard]] Accessor Type::operator[](size_t element_index) const
{
    throw TypeError("indexing is only available on arrays!");
}

Reference::Reference(const Type* type, char* address)
    : type(type)
    , address(address)
{}

Reference Reference::operator[](const std::string& member_name) const
{
    auto* object_type = dynamic_cast<const ObjectType*>(this->type);
    if (object_type != nullptr)
    {
        const ObjectType::Member& member = object_type->at(member_name);
        return Reference(member.type.get(), this->address + member.offset);
    }
    else
    {
        throw TypeError("member access is only available on objects!");
    }
}

Reference Reference::operator[](size_t element_index) const
{
    auto* array_type = dynamic_cast<const ArrayType*>(this->type);
    if (array_type != nullptr)
    {
        size_t element_offset = array_type->at(element_index);
        return Reference(array_type->element_type.get(), this->address + element_offset);
    }
    else
    {
        throw TypeError("indexing is only available on arrays!");
    }
}


Accessor::Accessor(const Type* origin, const Type* type, size_t offset)
    : origin(origin)
    , type(type)
    , offset(offset)
{}

Accessor Accessor::operator[](const std::string& member_name) const
{
    auto* object_type = dynamic_cast<const ObjectType*>(this->type);
    if (object_type != nullptr)
    {
        const ObjectType::Member& member = object_type->at(member_name);
        return Accessor(this->origin, member.type.get(), this->offset + member.offset);
    }
    else
    {
        throw TypeError("member access is only available on objects!");
    }
}

Accessor Accessor::operator[](size_t element_index) const
{
    auto* array_type = dynamic_cast<const ArrayType*>(this->type);
    if (array_type != nullptr)
    {
        size_t element_offset = array_type->at(element_index);
        return Accessor(this->origin, array_type->element_type.get(), this->offset + element_offset);
    }
    else
    {
        throw TypeError("indexing is only available on arrays!");
    }
}


ArrayType::ArrayType(std::shared_ptr<const Type> element_type, size_t length)
    : element_type(std::move(element_type))
    , element_size(align(this->element_type->alignment(), this->element_type->size()))
    , length(length)
{}

size_t ArrayType::size() const
{
    return this->element_size * this->length;
}

size_t ArrayType::alignment() const
{
    return this->element_type->alignment();
}

void ArrayType::construct(char *address) const
{
    for (size_t i = 0; i < this->length; ++i)
    {
        this->element_type->construct(address + this->element_size * i);
    }
}

void ArrayType::destroy(char *address) const
{
    for (size_t i = 0; i < this->length; ++i)
    {
        this->element_type->destroy(address + this->element_size * (this->length - i - 1));
    }
}

void ArrayType::emit(code::Cursor<code::Declaration> cursor) const
{
    this->element_type->emit(cursor);
    cursor.target.array_sizes.push_back(this->length);
}

Accessor ArrayType::operator[](size_t element_index) const
{
    size_t offset = this->at(element_index);
    return Accessor(this, this->element_type.get(), offset);
}

size_t ArrayType::at(size_t element_index) const
{
    if (element_index >= this->length)
    {
        throw IndexError("out of bounds index " + std::to_string(element_index));
    }
    return element_index * this->element_size;
}

ObjectType::Member::Member(
    std::shared_ptr<const Type> type,
    size_t offset,
    std::string&& name,
    code::Context<code::Declaration>* context
)
    : type(std::move(type))
    , offset(offset)
    , name(std::move(name))
    , context(context)
{}

void ObjectType::Member::emit(code::Cursor<code::Declaration> cursor) const
{
    this->type->emit(cursor);
    if (this->context != nullptr)
    {
        this->context->apply(cursor);
    }
}

ObjectType::Builder::Builder(const ObjectType* base) : base(base)
{
    if (base != nullptr)
    {
        this->members = base->members;
        this->members_lookup = base->members_lookup;
        this->members_size = base->members_size;
    }
}

size_t ObjectType::Builder::embed(const ObjectType* other)
{
    if (other->members.empty())
    {
        return 0;
    }

    // First, get the offset since it's computed for us
    const Member& front = other->members.front();
    size_t offset = this->member(front);

    // Rest
    for (size_t i = 1; i < other->members.size(); ++i)
    {
        const Member& member = other->members.at(i);
        this->member(member);
    }

    return offset;
}

size_t ObjectType::Builder::member(const Member& member)
{
    return this->member(member.name, member.type, member.context);
}

size_t ObjectType::Builder::member(
    std::string member_name,
    std::shared_ptr<const Type> member_type,
    code::Context<code::Declaration>* member_context
) {
    // TODO: find a clean way to log if a member is hidden here
    size_t offset = align(this->members_size, member_type->alignment());
    this->members_size = offset + member_type->size();
    // Overwrite here; excludes always hide base class members, so we want to always point to child's member
    this->members_lookup[member_name] = this->members.size() - 1;
    this->members.emplace_back(std::move(member_type), offset, std::move(member_name), member_context);
    return offset;
}

ObjectType::ObjectType(Builder&& builder)
    : members(std::move(builder.members))
    , members_lookup(std::move(builder.members_lookup))
    , members_size(builder.members_size)
    , name(std::move(builder.name))
    , base(builder.base)
    , context(builder.context)
{}

size_t ObjectType::size() const
{
    return this->members_size;
}

size_t ObjectType::alignment() const
{
    return this->members.empty() ? 0 : this->members.front().type->alignment();
}

void ObjectType::construct(char *address) const
{
    std::for_each(this->members.begin(), this->members.end(), [address](const Member& member)
    {
        member.type->construct(address + member.offset);
    });
}

void ObjectType::destroy(char *address) const
{
    std::for_each(this->members.rbegin(), this->members.rend(), [address](const Member& member)
    {
        member.type->destroy(address + member.offset);
    });
}

void ObjectType::emit(code::Cursor<code::Declaration> cursor) const
{
    cursor.target.type = this->name;
    cursor.dependencies.emplace(this->name);
}

void ObjectType::emit(code::Cursor<code::Definition> cursor) const
{
    if (this->base != nullptr)
    {
        cursor.target.base_name.emplace(this->base->name);
        cursor.dependencies.emplace(this->base->name);
    }

    for (auto iterator = this->begin_self(); iterator != this->end(); ++iterator)
    {
        iterator->emit(cursor.into(cursor.target.append(iterator->name)));
    }

    if (this->context != nullptr)
    {
        this->context->apply(cursor);
    }
}

Accessor ObjectType::operator[](const std::string &member_name) const
{
    const Member& member = this->at(member_name);
    return Accessor(this, member.type.get(), member.offset);
}

const ObjectType::Member& ObjectType::at(const std::string& member_name) const
{
    MembersLookup::const_iterator find = this->members_lookup.find(member_name);
    if (find != this->members_lookup.end())
    {
        return this->members[find->second];
    }
    else
    {
        throw MemberError("no such member " + member_name);
    }
}

[[nodiscard]] ObjectType::Members::const_iterator ObjectType::begin() const
{
    return this->members.begin();
}

[[nodiscard]] ObjectType::Members::const_iterator ObjectType::begin_self() const
{
    auto iterator = this->members.begin();
    if (this->base != nullptr)
    {
        std::advance(iterator, this->base->members.size());
    }
    return iterator;
}

[[nodiscard]] ObjectType::Members::const_iterator ObjectType::end() const
{
    return this->members.end();
}

}
