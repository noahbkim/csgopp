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

using google::protobuf::io::CodedInputStream;
using csgopp::common::database::Delete;
using csgopp::common::database::Database;
using csgopp::common::database::DatabaseWithNameId;
using csgopp::error::GameError;
using csgopp::client::data_table::DataTable;

struct ServerClass
{
    using Id = uint16_t;

    Id id{};
    std::string name;
    DataTable* data_table{nullptr};
    ServerClass* base_class{nullptr};
    bool is_base_class{false};

private:
//    template<typename Callback>
//    void traverse(DataTable* data_table, Callback callback);
};

using ServerClassDatabase = DatabaseWithNameId<ServerClass, Delete<ServerClass>>;

//template<typename Callback>
//void ServerClass::visit(DataTable* data_table, Callback callback, const std::string& prefix)
//{
//    for (DataTable::Property* property : data_table->properties)
//    {
//        if (property->excluded())
//        {
//            continue;
//        }
//
//        auto* data_table_property = dynamic_cast<DataTable::DataTableProperty*>(property);
//        if (data_table_property != nullptr)
//        {
//            visit(
//                data_table_property->data_table,
//                callback,
//                join(prefix, data_table_property->name, data_table_property->collapsible()));
//        }
//        else
//        {
//            callback(prefix, property);
//        }
//    }
//}

}
