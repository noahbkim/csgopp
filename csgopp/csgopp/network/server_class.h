#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <absl/container/flat_hash_map.h>

#include "../network.h"
#include "send_table.h"

namespace csgopp::network
{

using google::protobuf::io::CodedInputStream;

struct ServerClass
{
    uint16_t id{};
    std::string name;
    SendTable* send_table{nullptr};
    std::vector<ServerClass*> base_classes;
    absl::flat_hash_map<std::string_view, SendTable::Property*> properties;

    ServerClass() = default;
    ServerClass(CodedInputStream& stream, const Database<SendTable>& database);
    void deserialize(CodedInputStream& stream, const Database<SendTable>& database);
};

}
