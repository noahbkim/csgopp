#include "data_table.h"

#include "../common/control.h"
#include "server_class.h"
#include "entity.h"

#define GUARD(CONDITION) if (!(CONDITION)) { return false; }
#define CAST(OTHER, TYPE, VALUE) auto* (OTHER) = dynamic_cast<const TYPE*>(VALUE); GUARD((OTHER) != nullptr);
#define EQUAL(OTHER, NAME) ((OTHER)->NAME == this->NAME)

namespace csgopp::client::data_table
{

using csgopp::error::GameError;
using csgopp::common::object::shared;
using csgopp::common::control::concatenate;
using csgopp::client::entity::PropertyArrayType;
using csgopp::client::entity::EntityType;
using csgopp::client::entity::Offset;

Property::Property(CSVCMsg_SendTable_sendprop_t&& data)
    : name(std::move(*data.mutable_var_name()))
    , flags(data.flags())
    , priority(data.priority())
    , offset(nullptr, this, 0)
{}

void Property::build(EntityType::Builder& builder)
{
    std::shared_ptr<const common::object::Type> materialized = this->materialize();
    this->offset.type = dynamic_cast<const entity::PropertyValueType*>(materialized.get());
    this->offset.offset = builder.member(this->name, materialized, this);
}

[[nodiscard]] bool Property::equals(const Property* other) const
{
    return false;
};

Int32Property::Int32Property(CSVCMsg_SendTable_sendprop_t&& data)
    : bits(data.num_bits())
    , Property(std::move(data))
{}

Property::Kind::T Int32Property::kind() const
{
    return Kind::INT32;
}

std::shared_ptr<const common::object::Type> Int32Property::materialize() const
{
    if (this->bits == 1)
    {
        return shared<entity::BoolType>();
    }
    else if (this->flags & Flags::UNSIGNED)
    {
        return shared<entity::UnsignedInt32Type>();
    }
    else
    {
        return shared<entity::SignedInt32Type>();
    }
}

bool Int32Property::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, Int32Property, other);
    return EQUAL(as, bits);
}

FloatProperty::FloatProperty(CSVCMsg_SendTable_sendprop_t&& data)
    : high_value(data.high_value())
    , low_value(data.low_value())
    , bits(data.num_bits())
    , Property(std::move(data))
{}

Property::Kind::T FloatProperty::kind() const
{
    return Kind::FLOAT;
}

std::shared_ptr<const common::object::Type> FloatProperty::materialize() const
{
    return shared<entity::FloatType>();
}

bool FloatProperty::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, FloatProperty, other);
    return EQUAL(as, high_value) && EQUAL(as, low_value) && EQUAL(as, bits);
}

Vector3Property::Vector3Property(CSVCMsg_SendTable_sendprop_t&& data)
    : high_value(data.high_value())
    , low_value(data.low_value())
    , bits(data.num_bits())
    , Property(std::move(data))
{}

Property::Kind::T Vector3Property::kind() const
{
    return Kind::VECTOR3;
}

std::shared_ptr<const common::object::Type> Vector3Property::materialize() const
{
    return shared<entity::Vector3Type>();
}

bool Vector3Property::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, Vector3Property, other);
    return true;
}

Vector2Property::Vector2Property(CSVCMsg_SendTable_sendprop_t&& data)
    : high_value(data.high_value())
    , low_value(data.low_value())
    , bits(data.num_bits())
    , Property(std::move(data))
{}

Property::Kind::T Vector2Property::kind() const
{
    return Kind::VECTOR2;
}

std::shared_ptr<const common::object::Type> Vector2Property::materialize() const
{
    return shared<entity::Vector2Type>();
}

bool Vector2Property::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, Vector2Property, other);
    return true;
}

Property::Kind::T StringProperty::kind() const
{
    return Kind::STRING;
}

std::shared_ptr<const common::object::Type> StringProperty::materialize() const
{
    return shared<entity::StringType>();
}

bool StringProperty::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, StringProperty, other);
    return true;
}

ArrayProperty::ArrayProperty(CSVCMsg_SendTable_sendprop_t&& data, Property* element)
    : element(element)
    , length(data.num_elements())
    , Property(std::move(data))
{}

Property::Kind::T ArrayProperty::kind() const
{
    return Kind::ARRAY;
}

std::shared_ptr<const common::object::Type> ArrayProperty::materialize() const
{
    return std::make_shared<PropertyArrayType>(this->element->materialize(), this->length);
}

bool ArrayProperty::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, ArrayProperty, other);
    return this->element->equals(as->element.get()) && EQUAL(as, length);
}

void DataTableProperty::apply(Cursor<Declaration> declaration) const
{
    if (this->data_table->is_array)
    {
        declaration.target.annotations.emplace_back("is array");
    }
}

DataTableProperty::DataTableProperty(CSVCMsg_SendTable_sendprop_t&& data)
    : Property(std::move(data))
{}

Property::Kind::T DataTableProperty::kind() const
{
    return Kind::DATA_TABLE;
}

std::shared_ptr<const common::object::Type> DataTableProperty::materialize() const
{
    if (this->data_table->is_array)
    {
        return this->data_table->materialize_array();
    }
    else
    {
        return this->data_table->materialize();
    }
}

void DataTableProperty::build(EntityType::Builder& builder)
{
    if (this->name == "baseclass")
    {
        // Do nothing; this is already handled
    }
    else if (this->collapsible())
    {
        // TODO: investigate whether this is correct, DT_OverlayVars is only non-baseclass example
        std::shared_ptr<const EntityType> collapsible_type = this->data_table->materialize();
        this->offset.offset = builder.embed(collapsible_type.get());
    }
    else
    {
        Property::build(builder);
    }
}

bool DataTableProperty::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, DataTableProperty, other);
    return EQUAL(as, data_table);
}

Int64Property::Int64Property(CSVCMsg_SendTable_sendprop_t&& data)
    : bits(data.num_bits())
    , Property(std::move(data))
{}

Property::Kind::T Int64Property::kind() const
{
    return Kind::INT64;
}

std::shared_ptr<const common::object::Type> Int64Property::materialize() const
{
    if (this->flags & Flags::UNSIGNED)
    {
        return shared<entity::UnsignedInt64Type>();
    }
    else
    {
        return shared<entity::SignedInt64Type>();
    }
}

bool Int64Property::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, Int64Property, other);
    return EQUAL(as, bits);
}

DataTable::DataTable(const CSVCMsg_SendTable& data)
    : name(data.net_table_name())
    , properties(data.props_size())
{}

std::shared_ptr<const PropertyArrayType> DataTable::materialize_array()
{
    std::shared_ptr<const common::object::Type> array_type = this->properties.at(0)->materialize();

    // We have to manually set offsets since we're not materializing
    for (size_t i = 0; i < this->properties.size(); ++i)
    {
        this->properties.at(i)->offset.type = dynamic_cast<const entity::PropertyValueType*>(array_type.get());
        this->properties.at(i)->offset.offset = i * array_type->size();
    }

    size_t array_size = this->properties.size();
    return std::make_shared<PropertyArrayType>(array_type, array_size);
}

// TODO: remove recursion
std::shared_ptr<const EntityType> DataTable::materialize()
{
    if (this->entity_type != nullptr)
    {
        return this->entity_type;
    }

    const EntityType* base{nullptr};
    if (this->server_class != nullptr && this->server_class->base_class != nullptr)
    {
        base = this->server_class->base_class->data_table->materialize().get();
    }

    EntityType::Builder builder(base);
    builder.name = this->name;
    builder.context = this;
    for (DataTable::Property* property : this->properties)
    {
        property->build(builder);
    }
    this->entity_type = std::make_shared<EntityType>(std::move(builder), this);
    return this->entity_type;
}

void DataTable::apply(Cursor<Definition> cursor) const
{
    if (this->server_class != nullptr)
    {
        cursor.target.aliases.emplace_back(this->server_class->name);
    }
}

/// Left pad with zeros to 3 characters
bool is_array_index(std::string_view name, size_t index)
{
    if (name.size() < 3)
    {
        return false;
    }

    size_t i{0};
    for (; i < 3; ++i)
    {
        if (name[name.size() - 1 - i] != '0' + index % 10)
        {
            return false;
        }

        index /= 10;
    }

    for (; i < name.size(); ++i)
    {
        if (index == 0 || name[name.size() - 1 - i] != '0' + index % 10)
        {
            return false;
        }

        index /= 10;
    }

    return true;
}

}
