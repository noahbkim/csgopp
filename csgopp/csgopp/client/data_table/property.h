#pragma once

#include <cstdint>

#include "../../common/lookup.h"
#include "../../error.h"
#include "netmessages.pb.h"

namespace csgopp::client::data_table
{
    struct DataTable;
}

namespace csgopp::client::data_table::property
{

using csgo::message::net::CSVCMsg_SendTable_sendprop_t;
using csgopp::error::GameError;

struct PropertyFlags
{
    using Type = int32_t;
    enum : Type
    {
        UNSIGNED = 1 << 0,
        COORDINATES = 1 << 1,
        NO_SCALE = 1 << 2,
        ROUND_DOWN = 1 << 3,
        ROUND_UP = 1 << 4,
        NORMAL = 1 << 5,
        EXCLUDE = 1 << 6,
        XYZ = 1 << 7,
        INSIDE_ARRAY = 1 << 8,
        PROXY_ALWAYS_YES = 1 << 9,
        IS_VECTOR_ELEMENT = 1 << 10,
        COLLAPSIBLE = 1 << 11,
        COORDINATE_MAP = 1 << 12,
        COORDINATE_MAP_LOW_PRECISION = 1 << 13,
        COORDINATE_MAP_INTEGRAL = 1 << 14,
        CELL_COORDINATE = 1 << 15,
        CELL_COORDINATE_LOW_PRECISION = 1 << 16,
        CELL_COORDINATE_INTEGRAL = 1 << 17,
        CHANGES_OFTEN = 1 << 18,
        VARIABLE_INTEGER = 1 << 19,
        SPECIAL = NO_SCALE
                | NORMAL
                | COORDINATES
                | COORDINATE_MAP
                | COORDINATE_MAP_LOW_PRECISION
                | COORDINATE_MAP_INTEGRAL
                | CELL_COORDINATE
                | CELL_COORDINATE_LOW_PRECISION
                | CELL_COORDINATE_INTEGRAL,
    };
};

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

    DataTable* origin;
    std::string name;
    Type::T type;
    Flags::Type flags;
    int32_t priority;

    Property(DataTable* origin, const CSVCMsg_SendTable_sendprop_t& data)
        : origin(origin)
        , name(data.var_name())
        , type(data.type())
        , flags(data.flags())
        , priority(data.priority()) {}

    virtual ~Property() = default;
    // [[nodiscard]] virtual Value* value() const = 0;

    [[nodiscard]] constexpr bool excluded() const
    {
        return this->flags & Flags::EXCLUDE;
    }

    [[nodiscard]] constexpr bool collapsible() const
    {
        return this->flags & Flags::COLLAPSIBLE;
    }

    template<typename T>
    T* as()
    {
        T* subclass = dynamic_cast<T*>(this);
        ASSERT(subclass != nullptr, "failed to cast down property!");
        return subclass;
    }

    template<typename T>
    const T* as() const
    {
        const T* subclass = dynamic_cast<T*>(this);
        ASSERT(subclass != nullptr, "failed to cast down property!");
        return subclass;
    }
};

struct Int32Property : public Property
{
    int32_t bits;

    Int32Property(DataTable* data_table, const CSVCMsg_SendTable_sendprop_t& data)
        : Property(data_table, data)
        , bits(data.num_bits()) {}
};

struct SignedInt32Property final : public Int32Property
{
    int32_t value;

    using Int32Property::Int32Property;
};

struct UnsignedInt32Property final : public Int32Property
{
    uint32_t value;

    using Int32Property::Int32Property;
};

struct FloatProperty final : public Property
{
    float high_value;
    float low_value;
    int32_t bits;

    explicit FloatProperty(DataTable* data_table, const CSVCMsg_SendTable_sendprop_t& data)
        : Property(data_table, data)
        , high_value(data.high_value())
        , low_value(data.low_value())
        , bits(data.num_bits()) {}
};

struct Vector3Property final : public Property
{
    using Property::Property;
};

struct Vector2Property final : public Property
{
    using Property::Property;
};

struct StringProperty final : public Property
{
    using Property::Property;
};

struct ArrayProperty final : public Property
{
    Property* element{nullptr};
    int32_t size;

    explicit ArrayProperty(DataTable* data_table, const CSVCMsg_SendTable_sendprop_t& data, Property* element)
        : Property(data_table, data)
        , element(element)
        , size(data.num_elements()) {}
};

struct DataTableProperty final : public Property
{
    DataTable* value;

    using Property::Property;
};

struct Int64Property : public Property
{
    int32_t bits;

    explicit Int64Property(DataTable* data_table, const CSVCMsg_SendTable_sendprop_t& data)
        : Property(data_table, data)
        , bits(data.num_bits()) {}
};

struct SignedInt64Property final : public Int64Property
{
    int64_t value;

    using Int64Property::Int64Property;
};

struct UnsignedInt64Property final : public Int64Property
{
    uint64_t value;

    using Int64Property::Int64Property;
};

}
