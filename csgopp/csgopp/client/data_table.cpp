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
using csgopp::client::entity::Offset;

Property::Property(CSVCMsg_SendTable_sendprop_t&& data)
    : name(std::move(*data.mutable_var_name()))
    , flags(data.flags())
    , priority(data.priority())
{}

void Property::build(EntityType::Builder& builder)
{
    std::shared_ptr<const PropertyType> materialized = this->materialize();
    this->offset.type = materialized.get();
    this->offset.offset = builder.member(this->name, materialized, this);
}

void Property::apply(Cursor<csgopp::common::code::Declaration> declaration) const
{
    declaration.target.annotations.emplace_back(
        concatenate(
            std::to_string(this->priority),
            " ",
            (this->flags & Flags::COLLAPSIBLE) ? "collapsible" : ""));
}

[[nodiscard]] bool Property::equals(const Property* other) const
{
    return false;
};

[[nodiscard]] constexpr bool Property::collapsible() const
{
    return this->flags & Flags::COLLAPSIBLE;
}

[[nodiscard]] constexpr bool Property::changes_often() const
{
    return this->flags & Flags::CHANGES_OFTEN;
}

Int32Property::Int32Property(CSVCMsg_SendTable_sendprop_t&& data)
    : bits(data.num_bits())
    , Property(std::move(data))
{}

Property::Type::T Int32Property::type() const
{
    return Type::INT32;
}

std::shared_ptr<const PropertyType> Int32Property::materialize() const
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

Property::Type::T FloatProperty::type() const
{
    return Type::FLOAT;
}

std::shared_ptr<const PropertyType> FloatProperty::materialize() const
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

bool FloatProperty::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, FloatProperty, other);
    return EQUAL(as, high_value) && EQUAL(as, low_value) && EQUAL(as, bits);
}

Property::Type::T Vector3Property::type() const
{
    return Type::VECTOR3;
}

std::shared_ptr<const PropertyType> Vector3Property::materialize() const
{
    return shared<entity::Vector3Type>();
}

bool Vector3Property::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, Vector3Property, other);
    return true;
}

Property::Type::T Vector2Property::type() const
{
    return Type::VECTOR2;
}

std::shared_ptr<const PropertyType> Vector2Property::materialize() const
{
    return shared<entity::Vector2Type>();
}

bool Vector2Property::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, Vector2Property, other);
    return true;
}

Property::Type::T StringProperty::type() const
{
    return Type::STRING;
}

std::shared_ptr<const PropertyType> StringProperty::materialize() const
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
    , size(data.num_elements())
    , Property(std::move(data))
{}

Property::Type::T ArrayProperty::type() const
{
    return Type::ARRAY;
}

std::shared_ptr<const PropertyType> ArrayProperty::materialize() const
{
    return std::make_shared<ArrayType>(this->element->materialize(), this->size);
}

bool ArrayProperty::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, ArrayProperty, other);
    return this->element->equals(as->element.get()) && EQUAL(as, size);
}

DataTableProperty::DataTableProperty(CSVCMsg_SendTable_sendprop_t&& data)
    : Property(std::move(data))
{}

Property::Type::T DataTableProperty::type() const
{
    return Type::DATA_TABLE;
}

std::shared_ptr<const PropertyType> DataTableProperty::materialize() const
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
        return;
    }

    if (this->collapsible())
    {
        // TODO: investigate whether this is correct, DT_OverlayVars is only non-baseclass example
        std::shared_ptr<const EntityType> collapsible_type = this->data_table->materialize();
        this->offset.offset = builder.embed(collapsible_type.get());
    }

    Property::build(builder);
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

Property::Type::T Int64Property::type() const
{
    return Type::INT64;
}

std::shared_ptr<const PropertyType> Int64Property::materialize() const
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

template<typename Callback>
void collect_properties_tail(
    DataTable* data_table,
    size_t offset,
    absl::flat_hash_set<std::pair<std::string_view, std::string_view>>& excludes,
    Callback callback);

template<typename Callback>
void collect_properties_head(
    DataTable* data_table,
    size_t offset,
    absl::flat_hash_set<std::pair<std::string_view, std::string_view>>& excludes,
    Callback callback,
    std::vector<std::pair<Property*, Offset>>& offsets)
{
    // TODO: this doesn't 100% reflect the recursion but I'm pretty sure it's fine
    for (const auto& item : data_table->excludes)
    {
        excludes.emplace(item);
    }

    for (DataTable::Property* property : data_table->properties)
    {
        if (excludes.contains(std::make_pair(data_table->name, property->name)))
        {
            continue;
        }

        auto* data_table_property = dynamic_cast<DataTable::DataTableProperty*>(property);
        if (data_table_property != nullptr)
        {
            if (data_table_property->collapsible())
            {
                collect_properties_head(
                    data_table_property->data_table,
                    offset + property->offset.offset,
                    excludes,
                    callback,
                    offsets);
            }
            else
            {
                collect_properties_tail(
                    data_table_property->data_table,
                    offset + property->offset.offset,
                    excludes,
                    callback);
            }
        }
        else
        {
            property->offset.priority = property->priority;
            offsets.emplace_back(property, property->offset.from(offset));
        }
    }
}

template<typename Callback>
void collect_properties_tail(
    DataTable* data_table,
    size_t offset,
    absl::flat_hash_set<std::pair<std::string_view, std::string_view>>& excludes,
    Callback callback)
{
    std::vector<std::pair<Property*, Offset>> offsets;
    collect_properties_head(data_table, offset, excludes, callback, offsets);
    for (auto& pair : offsets)
    {
        callback(pair.first, pair.second);
    }
}

template<typename Callback>
void collect_properties(DataTable* data_table, Callback callback)
{
    absl::flat_hash_set<std::pair<std::string_view, std::string_view>> excludes;
    collect_properties_tail(data_table, 0, excludes, callback);
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

    EntityType::Builder builder(base);
    builder.name = this->name;
    builder.context = this;

    for (DataTable::Property* property : this->properties)
    {
        property->build(builder);
    }

    auto type = std::make_shared<EntityType>(std::move(builder), this);

    std::vector<Property*> prioritized_properties;
    collect_properties(this, [&type, &prioritized_properties](Property* property, Offset& absolute)
    {
        type->prioritized.emplace_back(absolute, property->name);
        prioritized_properties.emplace_back(property);
    });

    size_t start = 0;
    bool more = true;
    for (size_t priority = 0; priority <= 64 || more; ++priority)
    {
        more = false;
        for (size_t i = start; i < type->prioritized.size(); ++i)
        {
            Property* property = prioritized_properties[i];
            if (property->priority == priority || priority == 64 && property->changes_often())
            {
                if (start != i)
                {
                    std::swap(prioritized_properties[start], prioritized_properties[i]);
                    std::swap(type->prioritized[start], type->prioritized[i]);
                }
                start += 1;
            }
            else if (property->priority > priority)
            {
                more = true;
            }
        }
    }

    this->entity_type = type;
    return this->entity_type;
}

void DataTable::apply(Cursor<Definition> cursor) const
{
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
