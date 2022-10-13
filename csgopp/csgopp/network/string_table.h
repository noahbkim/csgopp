#pragma once

#include <string>
#include <optional>
#include <absl/container/flat_hash_map.h>

#include "netmessages.pb.h"

namespace csgopp::network
{

struct StringTable
{
    using Index = int32_t;

    struct Entry
    {
        using Data = std::vector<uint8_t>;

        std::string string;
        Data data;
    };

    std::string name;
    std::vector<Entry*> entries;

    StringTable(const std::string& name, size_t entry_count);
    StringTable(std::string&& name, size_t entry_count);
};

}
