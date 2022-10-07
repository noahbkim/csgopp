#pragma once

#include <memory>
#include <vector>
#include <string_view>
#include <absl/container/flat_hash_map.h>

#include "../common/vector.h"
#include "send_table.h"
#include "server_class.h"

namespace csgopp::network
{

using csgopp::common::vector::Vector3;

struct Entity
{
    int32_t id;
    Vector3 position;
    ServerClass* server_class;
    absl::flat_hash_map<std::string_view, std::unique_ptr<SendTable::Value>> members;
};

}
