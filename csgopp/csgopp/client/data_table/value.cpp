#include "value.h"

namespace csgopp::client::data_table::value
{

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

void Int32Value::deserialize(BitStream& stream)
{
    if (this->property->flags & Property::Flags::VARIABLE_INTEGER)
    {
        // Regardless of signed or unsigned we read the same way
        stream.read_variable_int(&this->value);
    }
    else
    {
        // Regardless of signed or unsigned we read number of bits
        stream.read(&this->value, this->property->bits);
    }
}

void FloatValue::deserialize(BitStream& stream)
{

}

void Vector3Value::deserialize(BitStream& stream)
{

}

void Vector2Value::deserialize(BitStream& stream)
{

}

void StringValue::deserialize(BitStream& stream)
{

}

void ArrayValue::deserialize(BitStream& stream)
{

}

void DataTableValue::deserialize(BitStream& stream)
{

}

void Int64Value::deserialize(BitStream& stream)
{

}

}
