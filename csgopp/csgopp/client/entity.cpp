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

void BitCoordinateFloatType::update(char* address, BitStream& stream) const
{

}

std::string BitCoordinateFloatType::declare(const std::string& name) const
{
    return "float " + name;
}

void BitNormalFloatType::update(char* address, BitStream& stream) const
{

}

std::string BitNormalFloatType::declare(const std::string& name) const
{
    return "float " + name;
}

void Vector3Type::update(char* address, BitStream& stream) const
{

}

std::string Vector3Type::declare(const std::string& name) const
{
    return "Vector3 " + name;
}

void Vector2Type::update(char* address, BitStream& stream) const
{

}

std::string Vector2Type::declare(const std::string& name) const
{
    return "Vector2 " + name;
}

void StringType::update(char* address, BitStream& stream) const
{

}

std::string StringType::declare(const std::string& name) const
{
    return "std::string " + name;
}

}
