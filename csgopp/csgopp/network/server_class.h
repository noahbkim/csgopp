#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <absl/container/flat_hash_map.h>

#include "send_table.h"

namespace csgopp::network
{

using google::protobuf::io::CodedInputStream;

struct SendTable;

struct ServerClass
{
    using Id = uint16_t;

    Id id{};
    std::string name;
    SendTable* send_table{nullptr};
    std::vector<ServerClass*> base_classes;
    absl::flat_hash_map<std::string_view, SendTable::Property*> properties;
};

}
