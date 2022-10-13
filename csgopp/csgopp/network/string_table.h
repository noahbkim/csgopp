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

    // Deserialization details that need to persist for updates
    size_t capacity;
    bool data_fixed;
    size_t data_size_bits;

    explicit StringTable(const csgo::message::net::CSVCMsg_CreateStringTable& data);
    StringTable(std::string&& name, size_t size);
};

}
