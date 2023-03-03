#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <absl/container/flat_hash_map.h>

#include "../common/database.h"
#include "../common/object.h"
#include "../error.h"
#include "data_table.h"

namespace csgopp::client::server_class
{

using csgopp::client::data_table::DataTable;
using csgopp::common::database::Database;
using csgopp::common::database::DatabaseWithName;
using csgopp::common::database::Delete;
using csgopp::error::GameError;
using google::protobuf::io::CodedInputStream;

struct ServerClass
{
    using Index = uint16_t;

    Index index{};
    std::string name;
    DataTable* data_table{nullptr};
    ServerClass* base_class{nullptr};
};

using ServerClassDatabase = DatabaseWithName<ServerClass, Delete<ServerClass>>;

}
