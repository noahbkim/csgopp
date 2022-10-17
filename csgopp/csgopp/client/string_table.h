#pragma once

#include <string>
#include <optional>
#include <absl/container/flat_hash_map.h>

#include "../common/database.h"
#include "netmessages.pb.h"

namespace csgopp::client::string_table
{

using csgopp::common::database::Delete;
using csgopp::common::database::Database;
using csgopp::common::database::DatabaseWithName;

struct StringTable
{
    using Index = int32_t;

    /// \todo Optimize this into a single blob with two views
    struct Entry
    {
        std::string string;
        std::string data;
    };

    std::string name;
    Database<Entry, Delete<Entry>> entries;

    // Deserialization details that need to persist for updates
    size_t capacity{};
    bool data_fixed{};
    size_t data_size_bits{};

    explicit StringTable(const csgo::message::net::CSVCMsg_CreateStringTable& data);
    StringTable(std::string&& name, size_t size);
};

using StringTableDatabase = DatabaseWithName<StringTable, Delete<StringTable>>;

}
