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
    Database<Entry> entries;

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

struct StringTableDatabase : public DatabaseWithName<StringTable>
{
    using DatabaseWithName::DatabaseWithName;

    std::shared_ptr<StringTable> instance_baseline;
    std::shared_ptr<StringTable> user_info;

    void on_emplace(const std::shared_ptr<StringTable>& string_table)
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

    void emplace(std::shared_ptr<StringTable>&& string_table) override
    {
        this->on_emplace(string_table);
        DatabaseWithName::emplace(std::move(string_table));
    }

    void emplace(size_t index, std::shared_ptr<StringTable>&& string_table) override
    {
        this->on_emplace(string_table);
        DatabaseWithName::emplace(index, std::move(string_table));
    }
};

}
