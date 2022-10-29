#include "data_table.h"
#include "server_class.h"

#define GUARD(CONDITION) if (!(CONDITION)) { return false; }
#define CAST(OTHER, TYPE, VALUE) auto* OTHER = dynamic_cast<const TYPE*>(VALUE); GUARD(OTHER != nullptr);
#define EQUAL(OTHER, NAME) (OTHER->NAME == this->NAME)

namespace csgopp::client::data_table
{

using csgopp::error::GameError;
using csgopp::common::object::ArrayType;
using csgopp::common::object::shared;

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

std::shared_ptr<const csgopp::common::object::Type> DataTable::Int32Property::materialize() const
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

std::shared_ptr<const csgopp::common::object::Type> DataTable::FloatProperty::materialize() const
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

std::shared_ptr<const csgopp::common::object::Type> DataTable::Vector3Property::materialize() const
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

std::shared_ptr<const csgopp::common::object::Type> DataTable::Vector2Property::materialize() const
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

std::shared_ptr<const csgopp::common::object::Type> DataTable::StringProperty::materialize() const
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
    : element(std::move(element))
    , size(data.num_elements())
    , Property(std::move(data))
{}

DataTable::Property::Type::T DataTable::ArrayProperty::type() const
{
    return Type::ARRAY;
}

std::shared_ptr<const csgopp::common::object::Type> DataTable::ArrayProperty::materialize() const
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

std::shared_ptr<const csgopp::common::object::Type> DataTable::DataTableProperty::materialize() const
{
    return this->data_table->materialize();
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

std::shared_ptr<const csgopp::common::object::Type> DataTable::Int64Property::materialize() const
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

        if (auto* data_table_property = dynamic_cast<DataTableProperty*>(property))
        {
            if (data_table_property->data_table->is_array)
            {
                std::shared_ptr<const common::object::Type> array_type =
                    data_table_property->data_table->properties.at(0)->materialize();
                size_t array_size = data_table_property->data_table->properties.size();
                builder.member(property->name, std::make_shared<ArrayType>(array_type, array_size));
                continue;
            }
        }

        builder.member(property->name, property->materialize());
    }

    // TODO: prioritized

    this->entity_type = std::make_shared<EntityType>(std::move(builder), this);
    return this->entity_type;
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
