#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../common/reader.h"
#include "../network/string_table.h"

namespace csgopp::model::user
{

using csgopp::network::StringTable;

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

    void deserialize(const StringTable::Entry::Data& data);
};

}
