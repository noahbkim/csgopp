#pragma once

#include <cstdint>
#include <string>

#include "../common/database.h"
#include "../client/string_table.h"

namespace csgopp::client::user
{

using csgopp::common::database::DatabaseWithId;
using csgopp::common::database::Delete;
using csgopp::client::string_table::StringTable;

struct User
{
    using Id = int32_t;
    using Index = uint16_t;

    Index index{};

    uint64_t version{};
    uint64_t xuid{};
    std::string name{};
    Id id{};  // Canonically user_id
    std::string guid{};

    uint32_t friends_id{};
    std::string friends_name{};

    bool is_fake{};
    bool is_hltv{};

    uint32_t custom_files[4]{};
    uint8_t files_downloaded{};

    explicit User(Index index);
};

struct UserDatabase : public DatabaseWithId<User, Delete<User>>
{

};

}
