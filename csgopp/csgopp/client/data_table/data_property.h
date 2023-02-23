#pragma once

#include "netmessages.pb.h"
#include "property.h"
#include "data_type.h"

namespace csgopp::client::data_table::data_property
{

using csgo::message::net::CSVCMsg_SendTable_sendprop_t;
using csgopp::client::data_table::property::Property;
using csgopp::client::data_table::data_type::DataType;
using csgopp::common::object::ObjectType;
using csgopp::common::object::Type;

struct DataProperty : public Property
{
    /// \brief Construct a property from serialized sendtable data.
    ///
    /// \param data a protobuf container of sendtable property data.
    explicit DataProperty(CSVCMsg_SendTable_sendprop_t&& data)
        : Property(std::move(data))
    {
    }

    /// \brief Must be a ValueType
    [[nodiscard]] virtual std::shared_ptr<const DataType> data_type() const = 0;

    /// \brief Trivial upcast.
    [[nodiscard]] std::shared_ptr<const Type> type() const override
    {
        return this->data_type();
    }
};

/// \brief Represents an integer with maximum width 32 bits. Can be boolean.
struct Int32Property final : public DataProperty
{
    int32_t bits;

    explicit Int32Property(CSVCMsg_SendTable_sendprop_t&& data);

    [[nodiscard]] Kind::T kind() const override;

    [[nodiscard]] std::shared_ptr<const DataType> data_type() const override;

    [[nodiscard]] bool equals(const Property* other) const override;
};

/// \brief Represents a float.
struct FloatProperty final : public DataProperty
{
    float high_value;
    float low_value;
    int32_t bits;

    explicit FloatProperty(CSVCMsg_SendTable_sendprop_t&& data);

    [[nodiscard]] Kind::T kind() const override;

    [[nodiscard]] std::shared_ptr<const DataType> data_type() const override;

    [[nodiscard]] bool equals(const Property* other) const override;
};

/// \brief Represents a 3D floating point vector.
struct Vector3Property final : public DataProperty
{
    float high_value;
    float low_value;
    int32_t bits;

    explicit Vector3Property(CSVCMsg_SendTable_sendprop_t&& data);

    [[nodiscard]] Kind::T kind() const override;

    [[nodiscard]] std::shared_ptr<const DataType> data_type() const override;

    [[nodiscard]] bool equals(const Property* other) const override;
};

/// \brief Represents a 2D floating point vector.
struct Vector2Property final : public DataProperty
{
    float high_value;
    float low_value;
    int32_t bits;

    explicit Vector2Property(CSVCMsg_SendTable_sendprop_t&& data);

    [[nodiscard]] Kind::T kind() const override;

    [[nodiscard]] std::shared_ptr<const DataType> data_type() const override;

    [[nodiscard]] bool equals(const Property* other) const override;
};

/// \brief Represents a string of arbitrary length.
struct StringProperty final : public DataProperty
{
    using DataProperty::DataProperty;

    [[nodiscard]] Kind::T kind() const override;

    [[nodiscard]] std::shared_ptr<const DataType> data_type() const override;

    [[nodiscard]] bool equals(const Property* other) const override;
};

/// \brief Represents a fixed-length array of a single other property type.
///
/// Array properties manage their own element type. It seems like arrays are
/// mostly for short arrays (pairs, coordinates, etc.).
struct ArrayProperty final : public DataProperty
{
    std::unique_ptr<DataProperty> element;
    int32_t length;

    ArrayProperty(CSVCMsg_SendTable_sendprop_t&& data, DataProperty* element);

    [[nodiscard]] Kind::T kind() const override;

    [[nodiscard]] std::shared_ptr<const DataType> data_type() const override;

    [[nodiscard]] bool equals(const Property* other) const override;
};

/// \brief Represents an integer with maximum width 64 bits.
struct Int64Property final : public DataProperty
{
    int32_t bits;

    explicit Int64Property(CSVCMsg_SendTable_sendprop_t&& data);

    [[nodiscard]] Kind::T kind() const override;

    [[nodiscard]] std::shared_ptr<const DataType> data_type() const override;

    [[nodiscard]] bool equals(const Property* other) const override;
};

}
