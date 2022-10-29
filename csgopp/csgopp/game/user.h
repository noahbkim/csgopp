#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../common/reader.h"
#include "../client/string_table.h"

namespace csgopp::game::user
{

using csgopp::client::string_table::StringTable;

struct User
{
    uint64_t version;
    uint64_t xuid;
    std::string name;
    uint32_t user_id;
    std::string guid;

    uint32_t friends_id;
    std::string friends_name;

    bool is_fake;
    bool is_hltv;

    uint32_t custom_files[4];
    uint8_t files_downloaded;

    void deserialize(const std::string& data);
};

}
