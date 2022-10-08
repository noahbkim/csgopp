#pragma once

#include <absl/container/flat_hash_map.h>
#include <google/protobuf/io/coded_stream.h>

#include "../common/lookup.h"
#include "data_table/property.h"
#include "data_table/value.h"
#include "csgopp/error.h"
#include "netmessages.pb.h"

namespace csgopp::network
{

using google::protobuf::io::CodedInputStream;
using csgopp::common::vector::Vector3;
using csgopp::common::vector::Vector2;
using csgopp::error::GameError;
using csgo::message::net::CSVCMsg_SendTable;
using csgo::message::net::CSVCMsg_SendTable_sendprop_t;

struct DataTable
{
    using Property = data_table::Property;
    using Value = data_table::Value;
    using Int32Property = data_table::Int32Property;
    using FloatProperty = data_table::FloatProperty;
    using Vector3Property = data_table::Vector3Property;
    using Vector2Property = data_table::Vector2Property;
    using StringProperty = data_table::StringProperty;
    using ArrayProperty = data_table::ArrayProperty;
    using DataTableProperty = data_table::DataTableProperty;
    using Int64Property = data_table::Int64Property;
    using Int32Value = data_table::Int32Value;
    using FloatValue = data_table::FloatValue;
    using Vector3Value = data_table::Vector3Value;
    using Vector2Value = data_table::Vector2Value;
    using StringValue = data_table::StringValue;
    using ArrayValue = data_table::ArrayValue;
    using DataTableValue = data_table::DataTableValue;
    using Int64Value = data_table::Int64Value;

public:
    std::string name;
    absl::flat_hash_map<std::string_view, Property*> properties;
    ServerClass* server_class{nullptr};

    DataTable() = default;
    explicit DataTable(const CSVCMsg_SendTable& data) : name(data.net_table_name()) {}
};

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
