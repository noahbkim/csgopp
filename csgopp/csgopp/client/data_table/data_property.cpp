#include "data_property.h"
#include "data_type.h"

#define GUARD(CONDITION) if (!(CONDITION)) { return false; }
#define CAST(OTHER, TYPE, VALUE) auto* (OTHER) = dynamic_cast<const TYPE*>(VALUE); GUARD((OTHER) != nullptr);
#define EQUAL(OTHER, NAME) ((OTHER)->NAME == this->NAME)

namespace csgopp::client::data_table::data_property
{

using csgopp::client::data_table::data_type::BoolType;
using csgopp::client::data_table::data_type::UnsignedInt32Type;
using csgopp::client::data_table::data_type::SignedInt32Type;
using csgopp::client::data_table::data_type::FloatType;
using csgopp::client::data_table::data_type::Vector3Type;
using csgopp::client::data_table::data_type::Vector2Type;
using csgopp::client::data_table::data_type::StringType;
using csgopp::client::data_table::data_type::UnsignedInt64Type;
using csgopp::client::data_table::data_type::SignedInt64Type;
using csgopp::client::data_table::data_type::DataArrayType;
using csgopp::common::object::shared;

Int32Property::Int32Property(CSVCMsg_SendTable_sendprop_t&& data)
    : bits(data.num_bits())
    , DataProperty(std::move(data))
{}

Property::Kind::T Int32Property::kind() const
{
    return Kind::INT32;
}

std::shared_ptr<const DataType> Int32Property::data_type() const
{
    if (this->bits == 1)
    {
        return shared<BoolType>();
    }
    else if (this->flags & Flags::UNSIGNED)
    {
        return shared<UnsignedInt32Type>();
    }
    else
    {
        return shared<SignedInt32Type>();
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
    , DataProperty(std::move(data))
{}

Property::Kind::T FloatProperty::kind() const
{
    return Kind::FLOAT;
}

std::shared_ptr<const DataType> FloatProperty::data_type() const
{
    return shared<FloatType>();
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
    , DataProperty(std::move(data))
{}

Property::Kind::T Vector3Property::kind() const
{
    return Kind::VECTOR3;
}

std::shared_ptr<const DataType> Vector3Property::data_type() const
{
    return shared<Vector3Type>();
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
    , DataProperty(std::move(data))
{}

Property::Kind::T Vector2Property::kind() const
{
    return Kind::VECTOR2;
}

std::shared_ptr<const DataType> Vector2Property::data_type() const
{
    return shared<Vector2Type>();
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

std::shared_ptr<const DataType> StringProperty::data_type() const
{
    return shared<StringType>();
}

bool StringProperty::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, StringProperty, other);
    return true;
}

ArrayProperty::ArrayProperty(CSVCMsg_SendTable_sendprop_t&& data, DataProperty* element)
    : element(element)
    , length(data.num_elements())
    , DataProperty(std::move(data))
{}

Property::Kind::T ArrayProperty::kind() const
{
    return Kind::ARRAY;
}

std::shared_ptr<const DataType> ArrayProperty::data_type() const
{
    return std::make_shared<DataArrayType>(this->element->data_type(), this->length);
}

bool ArrayProperty::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, ArrayProperty, other);
    return this->element->equals(as->element.get()) && EQUAL(as, length);
}

Int64Property::Int64Property(CSVCMsg_SendTable_sendprop_t&& data)
    : bits(data.num_bits())
    , DataProperty(std::move(data))
{}

Property::Kind::T Int64Property::kind() const
{
    return Kind::INT64;
}

std::shared_ptr<const DataType> Int64Property::data_type() const
{
    if (this->flags & Flags::UNSIGNED)
    {
        return shared<UnsignedInt64Type>();
    }
    else
    {
        return shared<SignedInt64Type>();
    }
}

bool Int64Property::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, Int64Property, other);
    return EQUAL(as, bits);
}

}
