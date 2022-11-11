#include "entity.h"
#include "data_table.h"

namespace csgopp::client::entity
{

using common::vector::Vector3;
using common::vector::Vector2;
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

namespace string
{
const size_t STRING_SIZE_BITS_MAX = 9;
const uint32_t STRING_SIZE_MAX = 1 << STRING_SIZE_BITS_MAX;
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

void BoolType::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "bool";
}

void BoolType::update(char* address, BitStream& stream, const Property* property) const
{
    uint8_t value;
    OK(stream.read(&value, 1));
    *reinterpret_cast<bool*>(address) = value;
}

void UnsignedInt32Type::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "uint32_t";
}

template<typename T>
inline void update_int_variable(char* address, BitStream& stream)
{
    stream.read_variable_unsigned_int(reinterpret_cast<T*>(address));
}

template<typename T, typename Underlying>
inline void update_int_fixed(char* address, BitStream& stream, const Property* property)
{
    const auto* int_property = reinterpret_cast<const Underlying*>(property);
    OK(int_property != nullptr);
    OK(stream.read(reinterpret_cast<T*>(address), int_property->bits));
}

void UnsignedInt32Type::update(char* address, BitStream& stream, const Property* property) const
{
    if (property->flags & Property::Flags::VARIABLE_INTEGER)
    {
        update_int_variable<uint32_t>(address, stream);
    }
    else
    {
        update_int_fixed<uint32_t, DataTable::Int32Property>(address, stream, property);
    }
}

void SignedInt32Type::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "int32_t";
}

void SignedInt32Type::update(char* address, BitStream& stream, const Property* property) const
{
    if (property->flags & Property::Flags::VARIABLE_INTEGER)
    {
        update_int_variable<int32_t>(address, stream);
    }
    else
    {
        update_int_fixed<int32_t, DataTable::Int32Property>(address, stream, property);
    }
}

void FloatType::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "float";
}

inline void update_float_coordinates(char* address, BitStream& stream)
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

inline void update_float_normal(char* address, BitStream& stream)
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
inline void update_float_coordinates_multiplayer(char* address, BitStream& stream)
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

inline void update_float_coordinates_multiplayer_integral(char* address, BitStream& stream)
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

template<typename Underlying, Precision P = Precision::Normal>
inline void update_float_cell_coordinates(char* address, BitStream& stream, const Property* property)
{
    float& value = *reinterpret_cast<float*>(address);

    const auto* float_property = dynamic_cast<const Underlying*>(property);
    OK(float_property != nullptr);

    uint32_t buffer;
    OK(stream.read(&buffer, float_property->bits));
    value = static_cast<float>(buffer);

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

template<typename Underlying>
inline void update_float_cell_coordinates_integral(char* address, BitStream& stream, const Property* property)
{
    float& value = *reinterpret_cast<float*>(address);

    const auto* float_property = dynamic_cast<const Underlying*>(property);
    OK(float_property != nullptr);

    uint32_t buffer;
    OK(stream.read(&buffer, float_property->bits));
    *reinterpret_cast<float*>(address) = static_cast<float>(buffer);
}

inline void update_float_no_scale(char* address, BitStream& stream)
{
    // Yes, it's a float, but our read only works with integral types; just use the same size
    OK(stream.read(reinterpret_cast<uint32_t*>(address), 32));
}

constexpr float interpolate(float a, float b, float x)
{
    return a + (b - a) * x;
}

template<typename Underlying>
inline void update_float_scaled(char* address, BitStream& stream, const Property* property)
{
    const auto* float_property = dynamic_cast<const DataTable::FloatProperty*>(property);
    OK(float_property != nullptr);

    uint32_t buffer;
    OK(stream.read(&buffer, float_property->bits));
    *reinterpret_cast<float*>(address) = interpolate(
        float_property->low_value,
        float_property->high_value,
        static_cast<float>(buffer) / static_cast<float>(1 << (float_property->bits - 1)));
}

template<typename Underlying = DataTable::FloatProperty>
inline void update_float(char* address, BitStream& stream, const Property* property)
{
    if (property->flags & Property::Flags::NO_SCALE)
    {
        update_float_no_scale(address, stream);
    }
    else
    {
        switch (property->flags & Property::Flags::FLOAT_FLAGS)
        {
            using Flags = Property::Flags;
            case 0:
                update_float_scaled<Underlying>(address, stream, property);
                break;
            case Flags::COORDINATES:
                update_float_coordinates(address, stream);
                break;
            case Flags::NORMAL:
                update_float_normal(address, stream);
                break;
            case Flags::COORDINATES_MULTIPLAYER:
                update_float_coordinates_multiplayer(address, stream);
                break;
            case Flags::COORDINATES_MULTIPLAYER_LOW_PRECISION:
                update_float_coordinates_multiplayer<Precision::Low>(address, stream);
                break;
            case Flags::COORDINATES_MULTIPLAYER_INTEGRAL:
                update_float_coordinates_multiplayer_integral(address, stream);
                break;
            case Flags::CELL_COORDINATES:
                update_float_cell_coordinates<Underlying>(address, stream, property);
                break;
            case Flags::CELL_COORDINATES_LOW_PRECISION:
                update_float_cell_coordinates<Underlying, Precision::Low>(address, stream, property);
                break;
            case Flags::CELL_COORDINATES_INTEGRAL:
                update_float_cell_coordinates_integral<Underlying>(address, stream, property);
                break;
            default:
                throw csgopp::error::GameError("invalid float flag set " + std::to_string(property->flags));
        }
    }
}

void FloatType::update(char* address, BitStream& stream, const Property* property) const
{
    update_float(address, stream, property);
}

void Vector3Type::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "Vector3";
}

void Vector3Type::update(char* address, BitStream& stream, const Property* property) const
{
    auto* value = reinterpret_cast<Vector3*>(address);
    update_float<DataTable::Vector3Property>(reinterpret_cast<char*>(&value->x), stream, property);
    update_float<DataTable::Vector3Property>(reinterpret_cast<char*>(&value->y), stream, property);

    if (property->flags & Property::Flags::NORMAL)
    {
        float magnitude = value->x * value->x + value->y * value->y;
        if (magnitude < 1)
        {
            value->z = std::sqrt(1 - magnitude);
        }
        else
        {
            value->z = 0;
        };
    }
    else
    {
        update_float<DataTable::Vector3Property>(reinterpret_cast<char*>(&value->z), stream, property);
    }
}

void Vector2Type::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "Vector2";
}

void Vector2Type::update(char* address, BitStream& stream, const Property* property) const
{
    auto* value = reinterpret_cast<Vector3*>(address);
    update_float<DataTable::Vector3Property>(reinterpret_cast<char*>(&value->x), stream, property);
    update_float<DataTable::Vector3Property>(reinterpret_cast<char*>(&value->y), stream, property);
}

void StringType::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "std::string";
}

void StringType::update(char* address, BitStream& stream, const Property* property) const
{
    std::string& value = *reinterpret_cast<std::string*>(address);

    uint32_t size;
    stream.read(&size, string::STRING_SIZE_BITS_MAX);
    stream.read_string_from(value, std::min(string::STRING_SIZE_MAX, size));
}

void UnsignedInt64Type::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "uint64_t";
}

void UnsignedInt64Type::update(char* address, BitStream& stream, const Property* property) const
{
    if (property->flags & Property::Flags::VARIABLE_INTEGER)
    {
        update_int_variable<uint64_t>(address, stream);
    }
    else
    {
        update_int_fixed<uint64_t, DataTable::Int64Property>(address, stream, property);
    }
}

void SignedInt64Type::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "int64_t";
}

void SignedInt64Type::update(char* address, BitStream& stream, const Property* property) const
{
    if (property->flags & Property::Flags::VARIABLE_INTEGER)
    {
        update_int_variable<int64_t>(address, stream);
    }
    else
    {
        update_int_fixed<int64_t, DataTable::Int64Property>(address, stream, property);
    }
}

void PropertyArrayType::update(char* address, BitStream& stream, const Property* property) const
{
    const auto* array_property = dynamic_cast<const DataTable::ArrayProperty*>(property);
    OK(array_property != nullptr);

    // Count how many elements we're receiving
    uint8_t size_bits = common::bits::width(this->length);
    size_t data_length;
    OK(stream.read(&data_length, size_bits));

    // Inner type cannot be an object; this has to be a runtime invariant
    const auto* value_type = dynamic_cast<const PropertyValueType*>(this->element_type.get());
    OK(value_type != nullptr);

    for (size_t i = 0; i < data_length; ++i)
    {
        value_type->update(address + this->at(i), stream, array_property->element.get());
    }
}

EntityType::EntityType(Builder&& builder, const DataTable* data_table)
    : ObjectType(std::move(builder))
    , data_table(data_table)
{}

}
