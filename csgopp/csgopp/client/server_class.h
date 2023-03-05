#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <absl/container/flat_hash_map.h>

#include "../common/database.h"
#include "../error.h"
#include "data_table.h"

namespace csgopp::client::server_class
{

using csgopp::client::data_table::DataTable;
using csgopp::common::database::Database;
using csgopp::common::database::DatabaseWithName;
using csgopp::error::GameError;
using google::protobuf::io::CodedInputStream;

struct ServerClass
{
    using Index = uint16_t;

    Index index{};
    std::string name;
    std::shared_ptr<DataTable> data_table;
    std::shared_ptr<ServerClass> base_class;
};

using ServerClassDatabase = DatabaseWithName<ServerClass>;

}
