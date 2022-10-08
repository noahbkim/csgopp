#pragma once

#include <cstdint>

#include "../../common/lookup.h"
#include "../../error.h"
#include "../declare.h"
#include "netmessages.pb.h"

namespace csgopp::network::data_table
{

using csgopp::error::GameError;
using csgo::message::net::CSVCMsg_SendTable_sendprop_t;

struct PropertyFlags
{
    using Type = int32_t;
    enum : Type
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

struct PropertyType
{
    using T = int32_t;
    enum E : T
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

struct Property
{
    using Type = PropertyType;
    using Flags = PropertyFlags;

    DataTable* send_table;
    std::string name;
    Type::T type;

    Property(DataTable* send_table, const CSVCMsg_SendTable_sendprop_t& data)
        : send_table(send_table)
        , name(data.var_name())
        , type(data.type()) {}

    virtual ~Property() = default;
    // [[nodiscard]] virtual Value* value() const = 0;

    template<typename T>
    T& as() const { return *dynamic_cast<T*>(this); }
};

struct Int32Property final : public Property
{
    int32_t bits;

    explicit Int32Property(DataTable* send_table, const CSVCMsg_SendTable_sendprop_t& data)
        : Property(send_table, data)
        , bits(data.num_bits()) {}

//        Value* instantiate(CodedInputStream& stream) const override;
};

struct FloatProperty final : public Property
{
    float high_value;
    float low_value;
    int32_t bits;
    Flags::Type flags;

    explicit FloatProperty(DataTable* send_table, const CSVCMsg_SendTable_sendprop_t& data)
        : Property(send_table, data)
        , high_value(data.high_value())
        , low_value(data.low_value())
        , bits(data.num_bits())
        , flags(data.flags()) {}

//        Value* instantiate(CodedInputStream& stream) const override;
};

struct Vector3Property final : public Property
{
    Flags::Type flags;

    explicit Vector3Property(DataTable* send_table, const CSVCMsg_SendTable_sendprop_t& data)
        : Property(send_table, data)
        , flags(data.flags()) {}

//        Value* instantiate(CodedInputStream& stream) const override;
};

struct Vector2Property final : public Property
{
    explicit Vector2Property(DataTable* send_table, const CSVCMsg_SendTable_sendprop_t& data)
        : Property(send_table, data) {}

//        Value* instantiate(CodedInputStream& stream) const override;
};

struct StringProperty final : public Property
{
    explicit StringProperty(DataTable* send_table, const CSVCMsg_SendTable_sendprop_t& data)
        : Property(send_table, data) {}

//        Value* instantiate(CodedInputStream& stream) const override;
};

struct ArrayProperty final : public Property
{
    int32_t size;

    explicit ArrayProperty(DataTable* send_table, const CSVCMsg_SendTable_sendprop_t& data)
        : Property(send_table, data)
        , size(data.num_elements()) {}

//        Value* instantiate(CodedInputStream& stream) const override;
};

struct DataTableProperty final : public Property
{
    std::string key;

    explicit DataTableProperty(DataTable* send_table, const CSVCMsg_SendTable_sendprop_t& data)
        : Property(send_table, data)
        , key(data.dt_name()) {}

//        Value* instantiate(CodedInputStream& stream) const override;
};

struct Int64Property final : public Property
{
    int32_t bits;

    explicit Int64Property(DataTable* send_table, const CSVCMsg_SendTable_sendprop_t& data)
        : Property(send_table, data)
        , bits(data.num_bits()) {}

//        Value* instantiate(CodedInputStream& stream) const override;
};

}
