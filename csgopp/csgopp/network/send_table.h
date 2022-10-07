#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <variant>
#include <google/protobuf/io/coded_stream.h>

#include "csgopp/common/vector.h"
#include "csgopp/common/macro.h"
#include "csgopp/common/lookup.h"
#include "csgopp/error.h"
#include "netmessages.pb.h"

namespace csgopp::network
{

using google::protobuf::io::CodedInputStream;
using csgopp::common::vector::Vector3;
using csgopp::error::GameError;

struct SendTablePropertyType
{
    using Type = int32_t;
    enum : Type
    {
        INT32 = 0,
        FLOAT = 1,
        VECTOR3 = 2,
        VECTOR2 = 3,
        STRING = 4,
        ARRAY = 5,
        DATA_TABLE = 6,
        INT64 = 7,
    };
};

LOOKUP(describe_send_table_property_type, SendTablePropertyType::Type, const char*,
    CASE(SendTablePropertyType::INT32, "INT32")
    CASE(SendTablePropertyType::FLOAT, "FLOAT")
    CASE(SendTablePropertyType::VECTOR3, "VECTOR3")
    CASE(SendTablePropertyType::VECTOR2, "VECTOR2")
    CASE(SendTablePropertyType::STRING, "STRING")
    CASE(SendTablePropertyType::ARRAY, "ARRAY")
    CASE(SendTablePropertyType::DATA_TABLE, "DATA_TABLE")
    CASE(SendTablePropertyType::INT64, "INT64")
    DEFAULT(throw  GameError("unknown send table property type: " + std::to_string(key))));

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

struct SendTable
{
    struct Value
    {
        using Data = std::variant<
                Vector3,
                int32_t,
                std::vector<Value>,
                std::string,
                float>;

        Data data;

        template<typename T>
        explicit Value(T&& value) : data(value) {}

        template<typename T>
        T as() { static_assert(sizeof(T) == 0, "only specializations of as() may be used!"); }

        template<> Vector3& as() { return std::get<Vector3>(this->data); }
        template<> int32_t& as() { return std::get<int32_t>(this->data); }
        template<> std::vector<Value>& as() { return std::get<std::vector<Value>>(this->data); }
        template<> std::string& as() { return std::get<std::string>(this->data); }
        template<> float& as() { return std::get<float>(this->data); }

        template<> bool as() { return std::get<int32_t>(this->data) > 0; }
    };

    struct Property
    {
        // These have to be separate for us to do binary operations
        using Flags = int32_t;
        struct Flag
        {
            enum
            {
                Unsigned = 1 << 0,
                Coordinates = 1 << 1,
                NoScale = 1 << 2,
                RoundDown = 1 << 3,
                RoundUp = 1 << 4,
                Normal = 1 << 5,
                Exclude = 1 << 6,
                XYZ = 1 << 7,
                InsideArray = 1 << 8,
                ProxyAlwaysYes = 1 << 9,
                IsVectorElement = 1 << 10,
                Collapsible = 1 << 11,
                CoordinateMap = 1 << 12,
                CoordMpLowPrecision = 1 << 13,
                CoordMpIntegral = 1 << 14,
                CellCoord = 1 << 15,
                CellCoordLowPrecision = 1 << 16,
                CellCoordIntegral = 1 << 17,
                ChangesOften = 1 << 18,
                VarInt = 1 << 19,
                Special = NoScale | Normal
                          | Coordinates | CoordinateMap | CoordMpLowPrecision | CoordMpIntegral
                          | CellCoord | CellCoordLowPrecision | CellCoordIntegral,
            };
        };

        using Type = SendTablePropertyType;

        Type::Type type;
        std::string data_table_name;
        Flags flags;
        int32_t elements_count;
        float high_value;
        float low_value;
        int32_t bits_count;

        Value deserialize(CodedInputStream& stream);
        inline Value deserialize_float(CodedInputStream& stream);
        inline Value deserialize_float_bit_coordinates(CodedInputStream& stream);
        inline Value deserialize_float_bit_normal(CodedInputStream& stream);
        inline Value deserialize_float_bit_cell_coordinates(CodedInputStream& stream);
        inline Value deserialize_int32(CodedInputStream& stream);
        inline Value deserialize_vector2(CodedInputStream& stream);
        inline Value deserialize_vector3(CodedInputStream& stream);
        inline Value deserialize_array(CodedInputStream& stream);
        inline Value deserialize_string(CodedInputStream& stream);
    };

    std::string name;
    std::vector<Property> properties;

    SendTable() = default;
    explicit SendTable(CodedInputStream& stream);

    // Weird quirk: send_tables don't come with a count, the end is marked by an empty send_table with is_end(). This
    // behavior is confirmed by Valve. For now, I'm gonna do deserialization by hand in the corresponding advance_*()
    // so there's no callback for this empty packet.
    // @see https://github.com/ValveSoftware/csgo-demoinfo/blob/master/demoinfogo/demofiledump.cpp#L1433
    bool deserialize(CodedInputStream& stream);
    void deserialize(csgo::message::net::CSVCMsg_SendTable& data);
};

}
