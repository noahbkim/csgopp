#include "object.h"

namespace csgopp::common::object
{

void ValueType::emit(code::Cursor<code::Declaration>& cursor) const
{
    cursor.target.type = this->info().name();
}

void ValueType::emit(layout::Cursor& cursor) const
{
    cursor.note(this->info().name());
}

Accessor::Accessor(std::shared_ptr<const Type> type, size_t offset)
    : type(std::move(type))
    , offset(offset)
{
}

Accessor Accessor::operator[](const std::string& member_name) const
{
    auto* object_type = dynamic_cast<const ObjectType*>(this->type.get());
    if (object_type != nullptr)
    {
        const ObjectType::Member& member = object_type->at(member_name);
        return Accessor(member.type, this->offset + member.offset);
    }
    else
    {
        throw TypeError("member access is only available on objects!");
    }
}

Accessor Accessor::operator[](size_t element_index) const
{
    auto* array_type = dynamic_cast<const ArrayType*>(this->type.get());
    if (array_type != nullptr)
    {
        size_t element_offset = array_type->at(element_index);
        return Accessor(array_type->element_type, this->offset + element_offset);
    }
    else
    {
        throw TypeError("indexing is only available on arrays!");
    }
}

bool Accessor::is_equal(const Accessor& other) const
{
    return this->offset == other.offset && this->type->size() == other.type->size();
}

bool Accessor::is_subset_of(const Accessor& other) const
{
    return other.offset <= this->offset && this->type->size() <= other.type->size();
}

bool Accessor::is_strict_subset_of(const Accessor& other) const
{
    return other.offset <= this->offset && this->type->size() < other.type->size();
}

bool Accessor::is_superset_of(const Accessor& other) const
{
    return this->offset <= other.offset && other.type->size() <= this->type->size();
}

bool Accessor::is_strict_superset_of(const Accessor& other) const
{
    return this->offset <= other.offset && other.type->size() < this->type->size();
}

Reference Reference::operator[](const std::string& member_name) const
{
    auto* object_type = dynamic_cast<const ObjectType*>(this->type.get());
    if (object_type != nullptr)
    {
        const ObjectType::Member& member = object_type->at(member_name);
        return Reference(
            member.type,
            std::shared_ptr<char[]>(this->address, this->address.get() + member.offset)
        );
    }
    else
    {
        throw TypeError("member access is only available on objects!");
    }
}

Reference Reference::operator[](size_t element_index) const
{
    auto* array_type = dynamic_cast<const ArrayType*>(this->type.get());
    if (array_type != nullptr)
    {
        size_t element_offset = array_type->at(element_index);
        return Reference(
            array_type->element_type,
            std::shared_ptr<char[]>(this->address, this->address.get() + element_offset)
        );
    }
    else
    {
        throw TypeError("indexing is only available on arrays!");
    }
}

ConstReference ConstReference::operator[](const std::string& member_name) const
{
    auto* object_type = dynamic_cast<const ObjectType*>(this->type.get());
    if (object_type != nullptr)
    {
        const ObjectType::Member& member = object_type->at(member_name);
        return ConstReference(
            member.type,
            std::shared_ptr<const char[]>(this->address, this->address.get() + member.offset)
        );
    }
    else
    {
        throw TypeError("member access is only available on objects!");
    }
}

ConstReference ConstReference::operator[](size_t element_index) const
{
    auto* array_type = dynamic_cast<const ArrayType*>(this->type.get());
    if (array_type != nullptr)
    {
        size_t element_offset = array_type->at(element_index);
        return ConstReference(
            array_type->element_type,
            std::shared_ptr<const char[]>(this->address, this->address.get() + element_offset)
        );
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
{
}

size_t ArrayType::size() const
{
    return this->element_size * this->length;
}

size_t ArrayType::alignment() const
{
    return this->element_type->alignment();
}

void ArrayType::construct(char* address) const
{
    for (size_t i = 0; i < this->length; ++i)
    {
        this->element_type->construct(address + this->element_size * i);
    }
}

void ArrayType::destroy(char* address) const
{
    for (size_t i = 0; i < this->length; ++i)
    {
        this->element_type->destroy(address + this->element_size * (this->length - i - 1));
    }
}

void ArrayType::emit(code::Cursor<code::Declaration>& cursor) const
{
    this->element_type->emit(cursor);
    cursor.target.array_sizes.push_back(this->length);
}

void ArrayType::emit(layout::Cursor& cursor) const
{
    for (size_t i = 0; i < this->length; ++i)
    {
        size_t relative = this->element_size * i;
        cursor.write(std::to_string(i), relative);
        layout::Cursor indented(cursor.indent(relative));
        this->element_type->emit(indented);
    }
}

size_t ArrayType::at(size_t element_index) const
{
    if (element_index >= this->length)
    {
        throw IndexError("out of bounds index " + std::to_string(element_index));
    }
    return element_index * this->element_size;
}

void ArrayType::represent(const char* address, std::ostream& out) const
{
    out << "[";
    for (size_t i = 0; i + 1 < this->length; ++i)  // Skip last, don't underflow by subtracting one
    {
        this->element_type->represent(address + this->at(i), out);
        out << ", ";
    }
    out << "]";
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
{
}

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
        return this->members_size;
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
)
{
    // TODO: find a clean way to log if a member is hidden here
    size_t offset = align(this->members_size, member_type->alignment());
    this->members_size = offset + member_type->size();
    // Overwrite here; excludes always hide base class members, so we want to always point to child's member
    this->members_lookup[member_name] = this->members.size();
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
{
}

size_t ObjectType::size() const
{
    return this->members_size;
}

size_t ObjectType::alignment() const
{
    return this->members.empty() ? 0 : this->members.front().type->alignment();
}

void ObjectType::construct(char* address) const
{
    std::for_each(
        this->members.begin(), this->members.end(), [address](const Member& member)
        {
            member.type->construct(address + member.offset);
        }
    );
}

void ObjectType::destroy(char* address) const
{
    std::for_each(
        this->members.rbegin(), this->members.rend(), [address](const Member& member)
        {
            member.type->destroy(address + member.offset);
        }
    );
}

void ObjectType::emit(code::Cursor<code::Declaration>& cursor) const
{
    cursor.target.type = this->name;
    cursor.dependencies.emplace(this->name);
}

void ObjectType::emit(code::Cursor<code::Definition>& cursor) const
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

void ObjectType::emit(layout::Cursor& cursor) const
{
    for (const Member& member: this->members)
    {
        cursor.write(member.name, member.offset);
        layout::Cursor indented(cursor.indent(member.offset));
        member.type->emit(indented);
    }
}

const ObjectType::Member& ObjectType::at(const std::string& member_name) const
{
    MembersLookup::const_iterator find = this->members_lookup.find(member_name);
    if (find != this->members_lookup.end())
    {
        // Can be left unchecked; we create the indices
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

void ObjectType::represent(const char* address, std::ostream& out) const
{
    out << "{";
    for (size_t i = 0; i < this->members.size(); ++i)
    {
        const Member& member = this->members.at(i);
        out << "\"" << member.name << "\"" << ": ";
        member.type->represent(address + member.offset, out);
        if (i + 1 != this->members_size)
        {
            out << ", ";
        }
    }
    out << "}";
}

std::ostream& operator<<(std::ostream& out, const Reference& reference)
{
    reference.type->represent(reference.address.get(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, const ConstReference& reference)
{
    reference.type->represent(reference.address.get(), out);
    return out;
}

}
