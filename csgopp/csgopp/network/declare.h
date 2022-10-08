#pragma once

#include <absl/container/flat_hash_map.h>

// Forward declarations; there are a lot of circular references
namespace csgopp::network
{
    struct SendTable;
    struct ServerClass;
    struct Network;
}
