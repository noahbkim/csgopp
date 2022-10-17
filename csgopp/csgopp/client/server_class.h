#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <absl/container/flat_hash_map.h>

#include "../common/database.h"
#include "data_table.h"

namespace csgopp::client::server_class
{

using google::protobuf::io::CodedInputStream;
using csgopp::common::database::Delete;
using csgopp::common::database::DatabaseWithName;
using csgopp::common::database::DatabaseWithNameId;
using csgopp::client::data_table::DataTable;

struct ServerClass
{
    using Id = uint16_t;
    using PropertyDatabase = DatabaseWithName<DataTable::Property>;

    Id id{};
    std::string name;
    DataTable* data_table{nullptr};
    std::vector<ServerClass*> base_classes;
    PropertyDatabase properties;
};

using ServerClassDatabase = DatabaseWithNameId<ServerClass, Delete<ServerClass>>;

}
