#pragma once

#include <type_traits>
#include <string>
#include <string_view>

namespace object
{

template<typename T>
struct dependent_false : std::false_type {};

static size_t _concatenate_size(std::string_view first)
{
    return first.size();
}

static size_t _concatenate_size(std::string_view arg, auto... args)
{
    return arg.size() + _concatenate_size(args...);
}

static void _concatenate_append(std::string& result, std::string_view arg)
{
    result += arg;
}

static void _concatenate_append(std::string& result, std::string_view arg, auto... args)
{
    result += arg;
    _concatenate_append(result, args...);
}

std::string concatenate(auto... args)
{
    std::string result;
    result.reserve(_concatenate_size(args...));
    _concatenate_append(result, args...);
    return result;
}

}
