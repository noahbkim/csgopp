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
using csgopp::client::entity::ArrayType;
using csgopp::client::entity::EntityType;

DataTable::Property::Property(CSVCMsg_SendTable_sendprop_t&& data)
    : name(std::move(*data.mutable_var_name()))
    , flags(data.flags())
    , priority(data.priority())
{}

DataTable::Int32Property::Int32Property(CSVCMsg_SendTable_sendprop_t&& data)
    : bits(data.num_bits())
    , Property(std::move(data))
{}

DataTable::Property::Type::T DataTable::Int32Property::type() const
{
    return Type::INT32;
}

std::shared_ptr<const PropertyType> DataTable::Int32Property::materialize() const
{
    if (this->flags & Flags::UNSIGNED)
    {
        if (this->flags & Flags::VARIABLE_INTEGER)
        {
            return shared<entity::UnsignedInt32Type<entity::Compression::Variable>>();
        }
        else
        {
            return shared<entity::UnsignedInt32Type<entity::Compression::Fixed>>();
        }
    }
    else
    {
        if (this->flags & Flags::VARIABLE_INTEGER)
        {
            return shared<entity::SignedInt32Type<entity::Compression::Variable>>();
        }
        else
        {
            return shared<entity::SignedInt32Type<entity::Compression::Fixed>>();
        }
    }
}

bool DataTable::Int32Property::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, Int32Property, other);
    return EQUAL(as, bits);
}

DataTable::FloatProperty::FloatProperty(CSVCMsg_SendTable_sendprop_t&& data)
    : high_value(data.high_value())
    , low_value(data.low_value())
    , bits(data.num_bits())
    , Property(std::move(data))
{}

DataTable::Property::Type::T DataTable::FloatProperty::type() const
{
    return Type::FLOAT;
}

std::shared_ptr<const PropertyType> DataTable::FloatProperty::materialize() const
{
    if (this->flags & Flags::COORDINATES)
    {
        return shared<entity::BitCoordinateFloatType>();
    }
    else if (this->flags & Flags::COORDINATES_MULTIPLAYER)
    {
        return shared<entity::MultiplayerBitCoordinateFloatType<entity::Precision::Normal>>();
    }
    else if (this->flags & Flags::COORDINATES_MULTIPLAYER_LOW_PRECISION)
    {
        return shared<entity::MultiplayerBitCoordinateFloatType<entity::Precision::Low>>();
    }
    else if (this->flags & Flags::COORDINATES_MULTIPLAYER_INTEGRAL)
    {
        return shared<entity::MultiplayerBitCoordinateFloatType<entity::Precision::Integral>>();
    }
    else if (this->flags & Flags::NO_SCALE)
    {
        return shared<entity::FloatType<entity::Serialization::Native>>();
    }
    else if (this->flags & Flags::NORMAL)
    {
        return shared<entity::BitNormalFloatType>();
    }
    else if (this->flags & Flags::CELL_COORDINATES)
    {
        return shared<entity::BitCellCoordinateFloatType<entity::Precision::Normal>>();
    }
    else if (this->flags & Flags::CELL_COORDINATES_LOW_PRECISION)
    {
        return shared<entity::BitCellCoordinateFloatType<entity::Precision::Low>>();
    }
    else if (this->flags & Flags::CELL_COORDINATES_INTEGRAL)
    {
        return shared<entity::BitCellCoordinateFloatType<entity::Precision::Integral>>();
    }
    else
    {
        return shared<entity::FloatType<entity::Serialization::Optimized>>();
    }
}

bool DataTable::FloatProperty::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, FloatProperty, other);
    return EQUAL(as, high_value) && EQUAL(as, low_value) && EQUAL(as, bits);
}

DataTable::Property::Type::T DataTable::Vector3Property::type() const
{
    return Type::VECTOR3;
}

std::shared_ptr<const PropertyType> DataTable::Vector3Property::materialize() const
{
    return shared<entity::Vector3Type>();
}

bool DataTable::Vector3Property::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, Vector3Property, other);
    return true;
}

DataTable::Property::Type::T DataTable::Vector2Property::type() const
{
    return Type::VECTOR2;
}

std::shared_ptr<const PropertyType> DataTable::Vector2Property::materialize() const
{
    return shared<entity::Vector2Type>();
}

bool DataTable::Vector2Property::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, Vector2Property, other);
    return true;
}

DataTable::Property::Type::T DataTable::StringProperty::type() const
{
    return Type::STRING;
}

std::shared_ptr<const PropertyType> DataTable::StringProperty::materialize() const
{
    return shared<entity::StringType>();
}

bool DataTable::StringProperty::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, StringProperty, other);
    return true;
}

DataTable::ArrayProperty::ArrayProperty(CSVCMsg_SendTable_sendprop_t&& data, Property* element)
    : element(element)
    , size(data.num_elements())
    , Property(std::move(data))
{}

DataTable::Property::Type::T DataTable::ArrayProperty::type() const
{
    return Type::ARRAY;
}

std::shared_ptr<const PropertyType> DataTable::ArrayProperty::materialize() const
{
    return std::make_shared<ArrayType>(this->element->materialize(), this->size);
}

bool DataTable::ArrayProperty::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, ArrayProperty, other);
    return this->element->equals(as->element.get()) && EQUAL(as, size);
}

DataTable::DataTableProperty::DataTableProperty(CSVCMsg_SendTable_sendprop_t&& data)
    : Property(std::move(data))
{}

DataTable::Property::Type::T DataTable::DataTableProperty::type() const
{
    return Type::DATA_TABLE;
}

std::shared_ptr<const PropertyType> DataTable::DataTableProperty::materialize() const
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

bool DataTable::DataTableProperty::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, DataTableProperty, other);
    return EQUAL(as, data_table);
}

DataTable::Int64Property::Int64Property(CSVCMsg_SendTable_sendprop_t&& data)
    : bits(data.num_bits())
    , Property(std::move(data))
{}

DataTable::Property::Type::T DataTable::Int64Property::type() const
{
    return Type::INT64;
}

std::shared_ptr<const PropertyType> DataTable::Int64Property::materialize() const
{
    if (this->flags & Flags::UNSIGNED)
    {
        if (this->flags & Flags::VARIABLE_INTEGER)
        {
            return shared<entity::UnsignedInt64Type<entity::Compression::Variable>>();
        }
        else
        {
            return shared<entity::UnsignedInt64Type<entity::Compression::Fixed>>();
        }
    }
    else
    {
        if (this->flags & Flags::VARIABLE_INTEGER)
        {
            return shared<entity::SignedInt64Type<entity::Compression::Variable>>();
        }
        else
        {
            return shared<entity::SignedInt64Type<entity::Compression::Fixed>>();
        }
    }
}

bool DataTable::Int64Property::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, Int64Property, other);
    return EQUAL(as, bits);
}

DataTable::DataTable(const CSVCMsg_SendTable& data)
    : name(data.net_table_name())
    , properties(data.props_size())
{}

struct AbsoluteProperty
{
    const DataTable::Property* property;
    entity::Offset offset;

    AbsoluteProperty(const DataTable::Property* property, entity::Offset offset)
        : property(property)
        , offset(offset)
    {}
};

void collect(
    DataTable* data_table,
    size_t offset,
    const DataTable::Excludes& excludes,
    std::vector<AbsoluteProperty>& properties
) {
    for (DataTable::Property* property : data_table->properties)
    {
        if (excludes.contains(std::make_pair(data_table->name, property->name)))
        {
            continue;
        }

        auto* data_table_property = dynamic_cast<DataTable::DataTableProperty*>(property);
        if (data_table_property != nullptr)
        {
            collect(data_table_property->data_table, offset + property->offset.offset, excludes, properties);
        }
        else
        {
            properties.emplace_back(property, entity::Offset(property->offset.type, offset + property->offset.offset));
        }
    }
}

std::vector<AbsoluteProperty> collect(DataTable* data_table)
{
    std::vector<AbsoluteProperty> properties;
    collect(data_table, 0, data_table->excludes, properties);
    return properties;
}

std::shared_ptr<const ArrayType> DataTable::materialize_array()
{
    std::shared_ptr<const PropertyType> array_type = this->properties.at(0)->materialize();

    // We have to manually set offsets since we're not materializing
    for (size_t i = 0; i < this->properties.size(); ++i)
    {
        this->properties.at(i)->offset.type = array_type.get();
        this->properties.at(i)->offset.offset = i * array_type->size();
    }

    size_t array_size = this->properties.size();
    return std::make_shared<ArrayType>(array_type, array_size);
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

    EntityType::Builder builder(this->name, base);

    for (DataTable::Property* property : this->properties)
    {
        // Skip base class
        if (property->name == "baseclass" && dynamic_cast<DataTableProperty*>(property) != nullptr)
        {
            continue;
        }

        if (property->collapsible())
        {
            auto object_type = std::dynamic_pointer_cast<const common::object::ObjectType>(property->materialize());
            builder.embed(object_type.get());
            continue;
        }

        std::shared_ptr<const PropertyType> materialized = property->materialize();
        property->offset.type = materialized.get();
        property->offset.offset = builder.member(property->name, materialized);
    }

    auto type = std::make_shared<EntityType>(std::move(builder), this);

    for (const AbsoluteProperty& property : collect(this))
    {
        type->prioritized.emplace_back(property.offset, property.property->name);
    }

    this->entity_type = type;
    return this->entity_type;
}

void DataTable::emit(Cursor<Definition> cursor) const
{
    if (this->entity_type->base != nullptr)
    {
        cursor.target.base_name.emplace(this->entity_type->base->name);
        cursor.dependencies.emplace(this->entity_type->base->name);
    }

    for (auto iterator = this->entity_type->begin_self(); iterator != this->entity_type->end(); ++iterator)
    {
        iterator->type->emit(cursor.into(cursor.target.append(iterator->name)));
    }

    for (auto& [a, b] : this->excludes)
    {
        cursor.target.annotations.emplace_back(concatenate("exclude: ", a, ".", b));
    }

    if (this->server_class != nullptr)
    {
        cursor.target.annotations.emplace_back(concatenate("server class: ", this->server_class->name));
    }
    else
    {
        cursor.target.annotations.emplace_back("server class: none");
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
