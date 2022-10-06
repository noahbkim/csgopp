#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <absl/container/flat_hash_map.h>

#include "send_table.h"

namespace csgopp::game
{

struct ServerClass
{
    struct FlattenedPropertyEntry
    {
        SendTable::Property* property;
        SendTable::Property* array_property;
        std::string name;
    };

    int32_t id;
    std::string name;
    int32_t data_table_id;
    std::string data_table_name;
    std::vector<ServerClass*> base_classes;
    std::vector<FlattenedPropertyEntry> flattened_property_entries;
    absl::flat_hash_map<std::string, size_t> flattened_property_entry_lookup;
};

}
