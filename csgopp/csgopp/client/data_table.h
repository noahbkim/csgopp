#pragma once

#include <variant>
#include <absl/container/flat_hash_map.h>
#include <google/protobuf/io/coded_stream.h>

#include "netmessages.pb.h"
#include "../error.h"
#include "../common/lookup.h"
#include "../common/database.h"
#include "../common/vector.h"
#include "../common/macro.h"
#include "../common/code.h"
#include "entity.h"

namespace csgopp::client::server_class
{

struct ServerClass;

}

namespace csgopp::client::data_table
{

using google::protobuf::io::CodedInputStream;
using csgo::message::net::CSVCMsg_SendTable;
using csgo::message::net::CSVCMsg_SendTable_sendprop_t;
using csgopp::error::GameError;
using csgopp::common::vector::Vector3;
using csgopp::common::vector::Vector2;
using csgopp::common::database::DatabaseWithName;
using csgopp::common::database::Delete;
using csgopp::common::database::NameTableMixin;
using csgopp::common::object::Annotator;
using csgopp::common::code::Definition;
using csgopp::common::code::Dependencies;
using csgopp::common::code::Declaration;
using csgopp::common::code::Cursor;
using csgopp::client::server_class::ServerClass;
using csgopp::client::entity::PropertyType;
using csgopp::client::entity::ArrayType;
using csgopp::client::entity::EntityType;
using csgopp::client::entity::Offset;

struct PropertyFlags
{
    using T = int32_t;
    enum : T
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
        COORDINATES_MULTIPLAYER = 1 << 12,
        COORDINATES_MULTIPLAYER_LOW_PRECISION = 1 << 13,
        COORDINATES_MULTIPLAYER_INTEGRAL = 1 << 14,
        CELL_COORDINATES = 1 << 15,
        CELL_COORDINATES_LOW_PRECISION = 1 << 16,
        CELL_COORDINATES_INTEGRAL = 1 << 17,
        CHANGES_OFTEN = 1 << 18,
        VARIABLE_INTEGER = 1 << 19,
    };
};

struct Property : Annotator
{
    struct Type
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

    using Flags = PropertyFlags;
    using Priority = int32_t;

    std::string name;
    Flags::T flags;
    Priority priority;
    Offset offset;

    explicit Property(CSVCMsg_SendTable_sendprop_t&& data);
    virtual ~Property() = default;

    [[nodiscard]] virtual Type::T type() const = 0;
    [[nodiscard]] virtual std::shared_ptr<const PropertyType> materialize() const = 0;
    virtual void build(EntityType::Builder& builder);

    void annotate(Cursor<Declaration> declaration) const override;

    [[nodiscard]] virtual bool equals(const Property* other) const;
    [[nodiscard]] constexpr bool excluded() const;
    [[nodiscard]] constexpr bool collapsible() const;
};

struct DataTable
{
    using Property = Property;

    struct Int32Property : public Property
    {
        int32_t bits;

        explicit Int32Property(CSVCMsg_SendTable_sendprop_t&& data);
        [[nodiscard]] Type::T type() const override;
        [[nodiscard]] std::shared_ptr<const PropertyType> materialize() const override;
        [[nodiscard]] bool equals(const Property* other) const override;
    };

    struct FloatProperty : public Property
    {
        float high_value;
        float low_value;
        int32_t bits;

        explicit FloatProperty(CSVCMsg_SendTable_sendprop_t&& data);
        [[nodiscard]] Type::T type() const override;
        [[nodiscard]] std::shared_ptr<const PropertyType> materialize() const override;
        [[nodiscard]] bool equals(const Property* other) const override;
    };

    struct Vector3Property : public Property
    {
        using Property::Property;
        [[nodiscard]] Type::T type() const override;
        [[nodiscard]] std::shared_ptr<const PropertyType> materialize() const override;
        [[nodiscard]] bool equals(const Property* other) const override;
    };

    struct Vector2Property : public Property
    {
        using Property::Property;
        [[nodiscard]] Type::T type() const override;
        [[nodiscard]] std::shared_ptr<const PropertyType> materialize() const override;
        [[nodiscard]] bool equals(const Property* other) const override;
    };

    struct StringProperty : public Property
    {
        using Property::Property;
        [[nodiscard]] Type::T type() const override;
        [[nodiscard]] std::shared_ptr<const PropertyType> materialize() const override;
        [[nodiscard]] bool equals(const Property* other) const override;
    };

    struct ArrayProperty : public Property
    {
        std::unique_ptr<Property> element;
        int32_t size;

        explicit ArrayProperty(CSVCMsg_SendTable_sendprop_t&& data, Property* element);
        [[nodiscard]] Type::T type() const override;
        [[nodiscard]] std::shared_ptr<const PropertyType> materialize() const override;
        [[nodiscard]] bool equals(const Property* other) const override;
    };

    struct DataTableProperty : public Property
    {
        DataTable* data_table{nullptr};

        // No constructor because data_table is set later on
        explicit DataTableProperty(CSVCMsg_SendTable_sendprop_t&& data);
        [[nodiscard]] Type::T type() const override;
        [[nodiscard]] std::shared_ptr<const PropertyType> materialize() const override;
        void build(EntityType::Builder& builder) override;

        [[nodiscard]] bool equals(const Property* other) const override;
    };

    struct Int64Property : public Property
    {
        int32_t bits;

        explicit Int64Property(CSVCMsg_SendTable_sendprop_t&& data);
        [[nodiscard]] Type::T type() const override;
        [[nodiscard]] std::shared_ptr<const PropertyType> materialize() const override;
        [[nodiscard]] bool equals(const Property* other) const override;
    };

    using PropertyDatabase = DatabaseWithName<Property, Delete<Property>>;
    using Excludes = absl::flat_hash_set<std::pair<std::string, std::string>>;

    std::string name;
    PropertyDatabase properties;  // pointer stability is very convenient
    ServerClass* server_class{nullptr};
    std::shared_ptr<const EntityType> entity_type;
    Excludes excludes;
    bool is_array{false};  // Whether we can make array when member

    DataTable() = default;
    explicit DataTable(const CSVCMsg_SendTable& data);

    std::shared_ptr<const EntityType> materialize();
    std::shared_ptr<const ArrayType> materialize_array();
    void emit(Cursor<Definition> cursor) const;
};

using DataTableDatabase = DatabaseWithName<DataTable, Delete<DataTable>>;

LOOKUP(describe, DataTable::Property::Type::T, const char*,
    CASE(DataTable::Property::Type::INT32, "INT32")
    CASE(DataTable::Property::Type::FLOAT, "FLOAT")
    CASE(DataTable::Property::Type::VECTOR3, "VECTOR3")
    CASE(DataTable::Property::Type::VECTOR2, "VECTOR2")
    CASE(DataTable::Property::Type::STRING, "STRING")
    CASE(DataTable::Property::Type::ARRAY, "ARRAY")
    CASE(DataTable::Property::Type::DATA_TABLE, "DATA_TABLE")
    CASE(DataTable::Property::Type::INT64, "INT64")
    DEFAULT(throw GameError("unknown send table property type: " + std::to_string(key))));

// https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/dt_send.cpp
bool is_array_index(std::string_view name, size_t index);

}
