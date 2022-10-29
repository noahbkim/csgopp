#pragma once

#include "../error.h"

namespace csgopp::common::control
{

using csgopp::error::GameError;

template<typename Map, typename Key, typename Error>
auto lookup(Map& first, Map& second, const Key& key, Error error)
{
    typename Map::const_iterator search = first.find(key);
    if (search == first.end())
    {
        search = second.find(key);
        if (search == second.end())
        {
            throw GameError(error());
        }
    }
    return search->second;
}

}
