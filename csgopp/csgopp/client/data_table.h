#pragma once

#include <absl/container/flat_hash_map.h>
#include <google/protobuf/io/coded_stream.h>

#include "../common/lookup.h"
#include "../common/database.h"
#include "../error.h"
#include "data_table/property.h"
#include "data_table/value.h"
#include "netmessages.pb.h"

namespace csgopp::client::server_class
{

struct ServerClass;

}

namespace csgopp::client::data_table
{

using google::protobuf::io::CodedInputStream;
using csgopp::common::vector::Vector3;
using csgopp::common::vector::Vector2;
using csgopp::common::database::DatabaseWithName;
using csgopp::common::database::Delete;
using csgopp::common::database::NameTableMixin;
using csgopp::error::GameError;
using csgopp::client::data_table::property::Property;
using csgopp::client::data_table::value::Value;
using csgopp::client::server_class::ServerClass;
using csgo::message::net::CSVCMsg_SendTable;
using csgo::message::net::CSVCMsg_SendTable_sendprop_t;

struct DataTable
{
    using Property = Property;
    using Value = Value;
    using Int32Property = data_table::property::Int32Property;
    using SignedInt32Property = data_table::property::SignedInt32Property;
    using UnsignedInt32Property = data_table::property::UnsignedInt32Property;
    using FloatProperty = data_table::property::FloatProperty;
    using Vector3Property = data_table::property::Vector3Property;
    using Vector2Property = data_table::property::Vector2Property;
    using StringProperty = data_table::property::StringProperty;
    using ArrayProperty = data_table::property::ArrayProperty;
    using DataTableProperty = data_table::property::DataTableProperty;
    using Int64Property = data_table::property::Int64Property;
    using SignedInt64Property = data_table::property::SignedInt64Property;
    using UnsignedInt64Property = data_table::property::UnsignedInt64Property;
    using Int32Value = data_table::value::Int32Value;
    using FloatValue = data_table::value::FloatValue;
    using Vector3Value = data_table::value::Vector3Value;
    using Vector2Value = data_table::value::Vector2Value;
    using StringValue = data_table::value::StringValue;
    using ArrayValue = data_table::value::ArrayValue;
    using DataTableValue = data_table::value::DataTableValue;
    using Int64Value = data_table::value::Int64Value;

    using PropertyDatabase = DatabaseWithName<Property, Delete<Property>>;

    std::string name;
    PropertyDatabase properties;
    ServerClass* server_class{nullptr};

    DataTable() = default;
    explicit DataTable(const CSVCMsg_SendTable& data) : name(data.net_table_name()) {}
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

}
