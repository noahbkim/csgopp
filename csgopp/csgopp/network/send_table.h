#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <string>
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
using csgopp::common::vector::Vector2;
using csgopp::error::GameError;
using csgo::message::net::CSVCMsg_SendTable_sendprop_t;

struct SendTablePropertyFlags
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

struct SendTable
{
    struct Value;

    struct Property
    {
        enum class Type : int32_t
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

        using Flags = SendTablePropertyFlags;

        SendTable* send_table;
        std::string name;

        Property(SendTable* send_table, const CSVCMsg_SendTable_sendprop_t& data)
            : send_table(send_table)
            , name(data.var_name()) {}

        virtual ~Property() = default;
        [[nodiscard]] virtual Type type() const = 0;
//        [[nodiscard]] virtual Value* value() const = 0;

        static std::unique_ptr<Property> deserialize(
            SendTable* send_table,
            const CSVCMsg_SendTable_sendprop_t& data);
    };

    struct Value
    {
        virtual ~Value() = default;
        [[nodiscard]] virtual Property* source() const = 0;
//        virtual void deserialize(CodedInputStream& stream) = 0;
    };

    struct Int32Property final : public Property
    {
        int32_t bits;

        explicit Int32Property(SendTable* send_table, const CSVCMsg_SendTable_sendprop_t& data)
            : Property(send_table, data)
            , bits(data.num_bits()) {}

        [[nodiscard]] Type type() const override { return Type::INT32; }
//        Value* instantiate(CodedInputStream& stream) const override;
    };

    struct Int32Value final : public Value
    {
        Int32Property* property;
        int32_t value;

        [[nodiscard]] Property* source() const override { return this->property; }
//        void update(CodedInputStream& stream) override;
    };

    struct FloatProperty final : public Property
    {
        float high_value;
        float low_value;
        int32_t bits;
        Flags::Type flags;

        explicit FloatProperty(SendTable* send_table, const CSVCMsg_SendTable_sendprop_t& data)
            : Property(send_table, data)
            , high_value(data.high_value())
            , low_value(data.low_value())
            , bits(data.num_bits())
            , flags(data.flags()) {}

        [[nodiscard]] Type type() const override { return Type::FLOAT; }
//        Value* instantiate(CodedInputStream& stream) const override;
    };

    struct FloatValue final : public Value
    {
        FloatProperty* property;
        float value;

        [[nodiscard]] Property* source() const override { return this->property; }
//        void update(CodedInputStream& stream) override;
    };

    struct Vector3Property final : public Property
    {
        Flags::Type flags;

        explicit Vector3Property(SendTable* send_table, const CSVCMsg_SendTable_sendprop_t& data)
            : Property(send_table, data)
            , flags(data.flags()) {}

        [[nodiscard]] Type type() const override { return Type::VECTOR3; }
//        Value* instantiate(CodedInputStream& stream) const override;
    };

    struct Vector3Value final : public Value
    {
        Vector3Property* property;
        Vector3 value;

        [[nodiscard]] Property* source() const override { return this->property; }
//        void update(CodedInputStream& stream) override;
    };

    struct Vector2Property final : public Property
    {
        explicit Vector2Property(SendTable* send_table, const CSVCMsg_SendTable_sendprop_t& data)
            : Property(send_table, data) {}

        [[nodiscard]] Type type() const override { return Type::VECTOR2; }
//        Value* instantiate(CodedInputStream& stream) const override;
    };

    struct Vector2Value final : public Value
    {
        Vector2Property* property;
        Vector2 value;

        [[nodiscard]] Property* source() const override { return this->property; }
//        void update(CodedInputStream& stream) override;
    };

    struct StringProperty final : public Property
    {
        explicit StringProperty(SendTable* send_table, const CSVCMsg_SendTable_sendprop_t& data)
            : Property(send_table, data) {}

        [[nodiscard]] Type type() const override { return Type::STRING; }
//        Value* instantiate(CodedInputStream& stream) const override;
    };

    struct StringValue final : public Value
    {
        StringProperty* property;
        std::string value;

        [[nodiscard]] Property* source() const override { return this->property; }
//        void update(CodedInputStream& stream) override;
    };

    struct ArrayProperty final : public Property
    {
        int32_t size;

        explicit ArrayProperty(SendTable* send_table, const CSVCMsg_SendTable_sendprop_t& data)
            : Property(send_table, data)
            , size(data.num_elements()) {}

        [[nodiscard]] Type type() const override { return Type::ARRAY; }
//        Value* instantiate(CodedInputStream& stream) const override;
    };

    struct ArrayValue final : public Value
    {
        Vector2Property* property;
        std::vector<Property*> value;

        [[nodiscard]] Property* source() const override { return this->property; }
//        void update(CodedInputStream& stream) override;
    };

    struct DataTableProperty final : public Property
    {
        std::string key;

        explicit DataTableProperty(SendTable* send_table, const CSVCMsg_SendTable_sendprop_t& data)
            : Property(send_table, data)
            , key(data.dt_name()) {}

        [[nodiscard]] Type type() const override { return Type::DATA_TABLE; }
//        Value* instantiate(CodedInputStream& stream) const override;
    };

    struct DataTableValue final : public Value
    {
        DataTableProperty* property;
        // TODO: ??

        [[nodiscard]] Property* source() const override { return this->property; }
//        void update(CodedInputStream& stream) override;
    };

    struct Int64Property final : public Property
    {
        int32_t bits;

        explicit Int64Property(SendTable* send_table, const CSVCMsg_SendTable_sendprop_t& data)
            : Property(send_table, data)
            , bits(data.num_bits()) {}

        [[nodiscard]] Type type() const override { return Type::INT64; }
//        Value* instantiate(CodedInputStream& stream) const override;
    };

    struct Int64Value final : public Value
    {
        Int64Property* property;
        int64_t value;

        [[nodiscard]] Property* source() const override { return this->property; }
//        void update(CodedInputStream& stream) override;
    };

public:
    std::string name;
    std::vector<std::unique_ptr<Property>> properties;

    SendTable() = default;
    explicit SendTable(CodedInputStream& stream);

    // Weird quirk: send_tables doesn't come with a count, the end is marked by an empty send_table with is_end(). This
    // behavior is confirmed by Valve. For now, I'm gonna do deserialization by hand in the corresponding advance_*()
    // so there's no callback for this empty packet.
    // @see https://github.com/ValveSoftware/csgo-demoinfo/blob/master/demoinfogo/demofiledump.cpp#L1433
    bool deserialize(CodedInputStream& stream);
    void deserialize(csgo::message::net::CSVCMsg_SendTable& data);
};

LOOKUP(describe_send_table_property_type, SendTable::Property::Type, const char*,
       CASE(SendTable::Property::Type::INT32, "INT32")
       CASE(SendTable::Property::Type::FLOAT, "FLOAT")
       CASE(SendTable::Property::Type::VECTOR3, "VECTOR3")
       CASE(SendTable::Property::Type::VECTOR2, "VECTOR2")
       CASE(SendTable::Property::Type::STRING, "STRING")
       CASE(SendTable::Property::Type::ARRAY, "ARRAY")
       CASE(SendTable::Property::Type::DATA_TABLE, "DATA_TABLE")
       CASE(SendTable::Property::Type::INT64, "INT64")
       DEFAULT(throw));

LOOKUP(deserialize_send_table_property_type, int32_t, SendTable::Property::Type,
       ENUM(SendTable::Property::Type::INT32)
       ENUM(SendTable::Property::Type::FLOAT)
       ENUM(SendTable::Property::Type::VECTOR3)
       ENUM(SendTable::Property::Type::VECTOR2)
       ENUM(SendTable::Property::Type::STRING)
       ENUM(SendTable::Property::Type::ARRAY)
       ENUM(SendTable::Property::Type::DATA_TABLE)
       ENUM(SendTable::Property::Type::INT64)
       DEFAULT(throw GameError("unknown send table property type: " + std::to_string(key))));

}
