#pragma once

#include <string>
#include <absl/container/flat_hash_map.h>

#include "netmessages.pb.h"

namespace csgopp::network
{

struct StringTable
{
    using Index = int32_t;

    std::string name;
    absl::flat_hash_map<Index, std::string> strings;

    explicit StringTable(const csgo::message::net::CSVCMsg_CreateStringTable& data);
};

}
