#include "entity.h"
#include "data_table.h"

namespace csgopp::client::entity
{

using csgopp::client::data_table::DataTable;

// https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/dt_common.h

namespace coordinates
{
const size_t FRACTIONAL_BITS_MP = 5;
const size_t FRACTIONAL_BITS_MP_LOW_PRECISION = 3;
const size_t DENOMINATOR = 1 << FRACTIONAL_BITS_MP;
const float RESOLUTION = 1.0 / DENOMINATOR;
const size_t DENOMINATOR_LOW_PRECISION = 1 << FRACTIONAL_BITS_MP_LOW_PRECISION;
const float RESOLUTION_LOW_PRECISION = 1.0 / DENOMINATOR_LOW_PRECISION;
const size_t INTEGER_BITS_MP = 11;
const size_t INTEGER_BITS = 14;
}

namespace normal
{
const size_t FRACTION_BITS = 11;
const size_t DENOMINATOR = 1 << (FRACTION_BITS - 1);
const float RESOLUTION = DENOMINATOR;
}

namespace data_table
{
const size_t string_size_bits_max = 9;
const size_t string_size_max = 1 << string_size_bits_max;
}

Offset::Offset(const struct PropertyValueType* type, const DataTable::Property* property, size_t offset)
    : type(type)
    , property(property)
    , offset(offset)
{}

Offset Offset::from(size_t parent) const
{
    Offset result(this->type, this->property, parent + this->offset);
    return result;
}

void BoolType::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "bool";
}

void BoolType::update(char* address, BitStream& stream, const Property* property) const
{
    uint8_t value;
    OK(stream.read(&value, 1));
    *reinterpret_cast<bool*>(address) = value;
}

void UnsignedInt32Type::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "uint32_t";
}

inline void update_unsigned_int32_variable(char* address, BitStream& stream, const Property* property)
{
    stream.read_variable_unsigned_int(reinterpret_cast<uint32_t*>(address));
}

inline void update_unsigned_int32(char* address, BitStream& stream, const Property* property)
{
    const auto* int32_property = reinterpret_cast<const DataTable::Int32Property*>(property);
    OK(int32_property != nullptr);
    OK(stream.read(reinterpret_cast<uint32_t*>(address), int32_property->bits));
}

void UnsignedInt32Type::update(char* address, BitStream& stream, const Property* property) const
{
    if (property->flags & Property::Flags::VARIABLE_INTEGER)
    {
        update_unsigned_int32_variable(address, stream, property);
    }
    else
    {
        update_unsigned_int32(address, stream, property);
    }
}

void SignedInt32Type::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "int32_t";
}

inline void update_signed_int32_variable(char* address, BitStream& stream, const Property* property)
{
    OK(stream.read_variable_signed_int(reinterpret_cast<int32_t*>(address)));
}

inline void update_signed_int32(char* address, BitStream& stream, const Property* property)
{
    const auto* int32_property = reinterpret_cast<const DataTable::Int32Property*>(property);
    OK(int32_property != nullptr);
    OK(stream.read(reinterpret_cast<int32_t*>(address), int32_property->bits));
}

void SignedInt32Type::update(char* address, BitStream& stream, const Property* property) const
{
    if (property->flags & Property::Flags::VARIABLE_INTEGER)
    {
        update_signed_int32_variable(address, stream, property);
    }
    else
    {
        update_signed_int32(address, stream, property);
    }
}

void FloatType::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "float";
}

inline void update_float_coordinates(char* address, BitStream& stream, const Property* property)
{
    // Always clear value to zero; if we get no data it means zero
    float& value = *reinterpret_cast<float*>(address);
    value = 0;

    uint8_t has_integral;
    OK(stream.read(&has_integral, 1));

    uint8_t has_fractional;
    OK(stream.read(&has_fractional, 1));

    if (has_integral || has_fractional)
    {
        uint8_t is_negative;
        OK(stream.read(&is_negative, 1));

        // INTEGER_BITS and FRACTIONAL_BITS_MP are < 16
        uint16_t buffer;

        if (has_integral)
        {
            OK(stream.read(&buffer, coordinates::INTEGER_BITS));
            value += static_cast<float>(buffer);
        }

        if (has_fractional)
        {
            OK(stream.read(&buffer, coordinates::FRACTIONAL_BITS_MP));
            value += static_cast<float>(buffer) * coordinates::RESOLUTION;
        }

        if (is_negative)
        {
            value = -value;
        }
    }
}

inline void update_float_normal(char* address, BitStream& stream, const Property* property)
{
    uint8_t is_negative;
    OK(stream.read(&is_negative, 1));

    uint16_t buffer;
    OK(stream.read(&buffer, normal::FRACTION_BITS));

    float& value = *reinterpret_cast<float*>(address);
    value = static_cast<float>(buffer) * normal::RESOLUTION;

    if (is_negative)
    {
        value = -value;
    }
}

template<Precision P = Precision::Normal>
inline void update_float_coordinates_multiplayer(char* address, BitStream& stream, const Property* property)
{
    // Always clear value to zero; if we get no data it means zero
    float& value = *reinterpret_cast<float*>(address);
    value = 0;

    uint8_t is_in_bounds;
    OK(stream.read(&is_in_bounds, 1));

    uint8_t has_integral;
    OK(stream.read(&has_integral, 1));

    uint8_t is_negative;
    OK(stream.read(&is_negative, 1));

    if (has_integral)
    {
        int16_t buffer;
        if (is_in_bounds)
        {
            OK(stream.read(&buffer, coordinates::INTEGER_BITS_MP));
        }
        else
        {
            OK(stream.read(&buffer, coordinates::INTEGER_BITS));
        }

        value += static_cast<float>(buffer);
    }

    if constexpr (P == Precision::Low)
    {
        uint8_t buffer;
        OK(stream.read(&buffer, coordinates::FRACTIONAL_BITS_MP_LOW_PRECISION));
        value += static_cast<float>(buffer) * coordinates::RESOLUTION_LOW_PRECISION;
    }
    else
    {
        uint8_t buffer;
        OK(stream.read(&buffer, coordinates::FRACTIONAL_BITS_MP));
        value += static_cast<float>(buffer) * coordinates::RESOLUTION;
    }

    if (is_negative)
    {
        value = -value;
    }
}

inline void update_float_coordinates_multiplayer_integral(char* address, BitStream& stream, const Property* property)
{
    // Always clear value to zero; if we get no data it means zero
    float& value = *reinterpret_cast<float*>(address);
    value = 0;

    uint8_t is_in_bounds;
    OK(stream.read(&is_in_bounds, 1));

    uint8_t is_non_zero;
    OK(stream.read(&is_non_zero, 1));

    if (is_non_zero)
    {
        uint8_t is_negative;
        OK(stream.read(&is_negative, 1));

        int16_t buffer;
        if (is_in_bounds)
        {
            OK(stream.read(&buffer, coordinates::INTEGER_BITS_MP));
        }
        else
        {
            OK(stream.read(&buffer, coordinates::INTEGER_BITS));
        }

        value = static_cast<float>(buffer);

        if (is_negative)
        {
            value = -value;
        }
    }
}

template<Precision P = Precision::Normal>
inline void update_float_cell_coordinates(char* address, BitStream& stream, const Property* property)
{
    // Always clear value to zero; if we get no data it means zero
    float& value = *reinterpret_cast<float*>(address);

    const auto* float_property = dynamic_cast<const DataTable::FloatProperty*>(property);

    uint32_t buffer;
    OK(stream.read(&buffer, float_property->bits));
    value += static_cast<float>(buffer);

    if constexpr (P == Precision::Low)
    {
        OK(stream.read(&buffer, coordinates::FRACTIONAL_BITS_MP_LOW_PRECISION));
        value += static_cast<float>(buffer) * coordinates::RESOLUTION_LOW_PRECISION;
    }
    else
    {
        OK(stream.read(&buffer, coordinates::FRACTIONAL_BITS_MP));
        value += static_cast<float>(buffer) * coordinates::RESOLUTION;
    }
}

inline void update_float_cell_coordinates_integral(char* address, BitStream& stream, const Property* property)
{
    const auto* float_property = dynamic_cast<const DataTable::FloatProperty*>(property);

    uint32_t buffer;
    OK(stream.read(&buffer, float_property->bits));
    *reinterpret_cast<float*>(address) = static_cast<float>(buffer);
}

inline void update_float_no_scale(char* address, BitStream& stream, const Property* property)
{
    // Yes, it's a float, but our read only works with integral types; just use the same size
    OK(stream.read(reinterpret_cast<uint32_t*>(address), 32));
}

constexpr float interpolate(float a, float b, float x)
{
    return a + (b - a) * x;
}

inline void update_float(char* address, BitStream& stream, const Property* property)
{
    const auto* float_property = dynamic_cast<const DataTable::FloatProperty*>(property);

    uint32_t buffer;
    OK(stream.read(&buffer, float_property->bits));
    *reinterpret_cast<float*>(address) = interpolate(
        float_property->low_value,
        float_property->high_value,
        static_cast<float>(buffer) / static_cast<float>(1 << (float_property->bits - 1)));
}

void FloatType::update(char* address, BitStream& stream, const Property* property) const
{
    if (property->flags & Property::Flags::NO_SCALE)
    {
        update_float_no_scale(address, stream, property);
        return;
    }

    switch (property->flags & Property::Flags::FLOAT_FLAGS)
    {
        using Flags = Property::Flags;
        case 0:
            update_float(address, stream, property);
            break;
        case Flags::COORDINATES:
            update_float_coordinates(address, stream, property);
            break;
        case Flags::NORMAL:
            update_float_normal(address, stream, property);
            break;
        case Flags::COORDINATES_MULTIPLAYER:
            update_float_coordinates_multiplayer(address, stream, property);
            break;
        case Flags::COORDINATES_MULTIPLAYER_LOW_PRECISION:
            update_float_coordinates_multiplayer<Precision::Low>(address, stream, property);
            break;
        case Flags::COORDINATES_MULTIPLAYER_INTEGRAL:
            update_float_coordinates_multiplayer_integral(address, stream, property);
            break;
        case Flags::CELL_COORDINATES:
            update_float_cell_coordinates(address, stream, property);
            break;
        case Flags::CELL_COORDINATES_LOW_PRECISION:
            update_float_cell_coordinates<Precision::Low>(address, stream, property);
            break;
        case Flags::CELL_COORDINATES_INTEGRAL:
            update_float_cell_coordinates_integral(address, stream, property);
            break;
        default:
            throw csgopp::error::GameError("invalid float flag set " + std::to_string(property->flags));
    }
}

void Vector3Type::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "Vector3";
}

void Vector3Type::update(char* address, BitStream& stream, const Property* property) const
{

}

void Vector2Type::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "Vector2";
}

void Vector2Type::update(char* address, BitStream& stream, const Property* property) const
{

}

void StringType::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "std::string";
}

void StringType::update(char* address, BitStream& stream, const Property* property) const
{

}

void UnsignedInt64Type::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "uint64_t";
}

void UnsignedInt64Type::update(char* address, BitStream& stream, const Property* property) const
{

}

void SignedInt64Type::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "int64_t";
}

void SignedInt64Type::update(char* address, BitStream& stream, const Property* property) const
{

}

void PropertyArrayType::update(char* address, BitStream& stream, const Property* property) const
{

}

EntityType::EntityType(Builder&& builder, const DataTable* data_table)
    : ObjectType(std::move(builder))
    , data_table(data_table)
{}

void EntityType::update(char* address, BitStream& stream, const Property* property) const
{

}

}
