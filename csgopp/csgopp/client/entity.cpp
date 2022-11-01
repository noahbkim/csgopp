#include "entity.h"

namespace csgopp::client::entity
{

// https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/dt_common.h

namespace coordinates
{
const size_t fractional_bits_mp = 5;
const size_t fractional_bits_mp_low_precision = 3;
const size_t denominator = 1 << fractional_bits_mp;
const float resolution = 1.0 / denominator;
const size_t denominator_low_precision = 1 << fractional_bits_mp_low_precision;
const float resolution_low_precision = 1.0 / denominator_low_precision;
const size_t integer_bits_mp = 11;
const size_t integer_bits = 14;
}

namespace normal
{
const size_t fraction_bits = 11;
const size_t denominator = 1 << (fraction_bits - 1);
const float resolution = denominator;
}

namespace data_table
{
const size_t string_size_bits_max = 9;
const size_t string_size_max = 1 << string_size_bits_max;
}

Offset::Offset(const struct PropertyType* type, size_t offset)
    : type(type)
    , offset(offset)
{}

Offset Offset::from(size_t parent) const
{
    Offset result(this->type, parent + this->offset);
    result.priority = this->priority;
    return result;
}

void BoolType::update(char* address, BitStream& stream) const
{

}

void BoolType::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "bool";
}

void BitCoordinateFloatType::update(char* address, BitStream& stream) const
{

}

void BitCoordinateFloatType::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "float";
}

void BitNormalFloatType::update(char* address, BitStream& stream) const
{

}

void BitNormalFloatType::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "float";
}

void Vector3Type::update(char* address, BitStream& stream) const
{

}

void Vector3Type::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "Vector3";
}

void Vector2Type::update(char* address, BitStream& stream) const
{

}

void Vector2Type::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "Vector2";
}

void StringType::update(char* address, BitStream& stream) const
{

}

void StringType::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "std::string";
}

EntityType::EntityType(Builder&& builder, const DataTable* data_table)
    : ObjectType(std::move(builder))
    , data_table(data_table)
{}

void EntityType::update(char* address, csgopp::common::bits::BitStream& stream) const
{
    // TODO
}

}
