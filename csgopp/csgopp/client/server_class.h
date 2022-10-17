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
    ServerClass* base_class{nullptr};
    PropertyDatabase properties;

    template<typename Callback>
    void traverse_properties(Callback callback)
    {
        for (const DataTable::Property* property : this->properties)
        {

        }
    }
};

using ServerClassDatabase = DatabaseWithNameId<ServerClass, Delete<ServerClass>>;

}
