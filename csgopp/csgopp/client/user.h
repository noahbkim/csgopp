#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../common/reader.h"
#include "../common/database.h"
#include "../client/string_table.h"

namespace csgopp::client::user
{

using csgopp::common::database::Database;
using csgopp::common::database::Delete;
using csgopp::client::string_table::StringTable;

struct User
{
    using Id = size_t;

    Id id{};

    uint64_t version{};
    uint64_t xuid{};
    std::string name{};
    uint32_t user_id{};
    std::string guid{};

    uint32_t friends_id{};
    std::string friends_name{};

    bool is_fake{};
    bool is_hltv{};

    uint32_t custom_files[4]{};
    uint8_t files_downloaded{};

    explicit User(Id id);

    void deserialize(const std::string& data);
};

using UserDatabase = Database<User, Delete<User>>;

}
