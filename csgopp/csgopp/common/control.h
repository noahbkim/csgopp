#pragma once

#include <optional>

namespace csgopp::common::control
{

template<typename Map>
std::optional<typename Map::value_type> at(Map& map, typename Map::key_type&& key)
{
    auto iterator = map.find(key);
    if (iterator == map.end())
    {
        return std::nullopt;
    }
    else
    {
        return std::make_optional(iterator->second);
    }
}

}
