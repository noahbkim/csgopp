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

template<typename... T>
void _concatenate(std::string& base, std::string_view next, T... args)
{
    base += next;
    _concatenate(base, args...);
}

// These have to be declared here because they're custom specializations
template<>
void _concatenate(std::string& base, std::string_view next);

template<typename... T>
std::string concatenate(std::string base, T... args)
{
    _concatenate(base, args...);
    return base;
}

template<>
std::string concatenate(std::string base);

}
