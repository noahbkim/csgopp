#include <type_traits>

namespace object
{

template<typename T>
struct dependent_false : std::false_type {};

namespace
{

size_t concatenate_size(std::string_view first)
{
    return first.size();
}

size_t concatenate_size(std::string_view arg, auto... args)
{
    return arg.size() + concatenate_size(args...);
}

void concatenate_append(std::string& result, std::string_view arg)
{
    result += arg;
}

void concatenate_append(std::string& result, std::string_view arg, auto... args)
{
    result += arg;
    concatenate_append(result, args...);
}

}

std::string concatenate(auto... args)
{
    std::string result;
    result.reserve(concatenate_size(args...));
    concatenate_append(result, args...);
    return result;
}

}
