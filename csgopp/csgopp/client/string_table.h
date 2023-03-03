#pragma once

#include <string>
#include <optional>
#include <absl/container/flat_hash_map.h>

#include "../common/database.h"
#include "netmessages.pb.h"

namespace csgopp::client::string_table
{

using csgopp::common::database::Database;
using csgopp::common::database::DatabaseWithName;
using csgopp::common::database::Delete;

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

    explicit StringTable(const csgo::message::net::CSVCMsg_CreateStringTable& data)
        : name(data.name())
        , entries(data.num_entries())
        , capacity(data.max_entries())
        , data_fixed(data.user_data_fixed_size())
        , data_size_bits(data.user_data_size_bits())
    {
    }

    StringTable(std::string&& name, size_t size)
        : name(std::move(name))
        , entries(size)
    {
    }
};

struct StringTableDatabase : public DatabaseWithName<StringTable, Delete<StringTable>>
{
    using DatabaseWithName::DatabaseWithName;

    StringTable* instance_baseline{nullptr};
    StringTable* user_info{nullptr};

    void on_emplace(StringTable* string_table)
    {
        if (string_table->name == "instancebaseline")
        {
            this->instance_baseline = string_table;
        }
        else if (string_table->name == "userinfo")
        {
            this->user_info = string_table;
        }
    }

    void emplace(StringTable* string_table) override
    {
        DatabaseWithName::emplace(string_table);
        this->on_emplace(string_table);
    }

    void emplace(size_t index, StringTable* string_table) override
    {
        DatabaseWithName::emplace(index, string_table);
        this->on_emplace(string_table);
    }
};

}
