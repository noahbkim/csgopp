#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <absl/container/flat_hash_map.h>

#include "data_table.h"

namespace csgopp::network
{

using google::protobuf::io::CodedInputStream;

struct DataTable;

struct ServerClass
{
    using Id = uint16_t;

    Id id{};
    std::string name;
    DataTable* data_table{nullptr};
    std::vector<ServerClass*> base_classes;
    absl::flat_hash_map<std::string_view, DataTable::Property*> properties;
};

}
