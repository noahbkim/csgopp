#pragma once

#include <absl/container/flat_hash_map.h>

namespace csgopp::network
{

template<typename Value>
using Database = absl::flat_hash_map<std::string_view, std::unique_ptr<Value>>;

}
