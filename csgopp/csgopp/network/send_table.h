#pragma once

#include <absl/container/flat_hash_map.h>
#include <google/protobuf/io/coded_stream.h>

#include "../common/lookup.h"
#include "send_table/property.h"
#include "send_table/value.h"
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

struct SendTable
{
    using Property = send_table::Property;
    using Value = send_table::Value;
    using Int32Property = send_table::Int32Property;
    using FloatProperty = send_table::FloatProperty;
    using Vector3Property = send_table::Vector3Property;
    using Vector2Property = send_table::Vector2Property;
    using StringProperty = send_table::StringProperty;
    using ArrayProperty = send_table::ArrayProperty;
    using DataTableProperty = send_table::DataTableProperty;
    using Int64Property = send_table::Int64Property;
    using Int32Value = send_table::Int32Value;
    using FloatValue = send_table::FloatValue;
    using Vector3Value = send_table::Vector3Value;
    using Vector2Value = send_table::Vector2Value;
    using StringValue = send_table::StringValue;
    using ArrayValue = send_table::ArrayValue;
    using DataTableValue = send_table::DataTableValue;
    using Int64Value = send_table::Int64Value;

public:
    std::string name;
    absl::flat_hash_map<std::string_view, Property*> properties;
    ServerClass* server_class{nullptr};

    SendTable() = default;
    explicit SendTable(const CSVCMsg_SendTable& data) : name(data.net_table_name()) {}
};

LOOKUP(describe, SendTable::Property::Type::T, const char*,
    CASE(SendTable::Property::Type::INT32, "INT32")
    CASE(SendTable::Property::Type::FLOAT, "FLOAT")
    CASE(SendTable::Property::Type::VECTOR3, "VECTOR3")
    CASE(SendTable::Property::Type::VECTOR2, "VECTOR2")
    CASE(SendTable::Property::Type::STRING, "STRING")
    CASE(SendTable::Property::Type::ARRAY, "ARRAY")
    CASE(SendTable::Property::Type::DATA_TABLE, "DATA_TABLE")
    CASE(SendTable::Property::Type::INT64, "INT64")
    DEFAULT(throw GameError("unknown send table property type: " + std::to_string(key))));

}
