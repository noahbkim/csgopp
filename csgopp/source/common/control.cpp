#include "control.h"

namespace csgopp::common::control
{

template<>
void _concatenate(std::string& base, std::string_view next)
{
    base += next;
}

template<>
std::string concatenate(std::string base)
{
    return base;
}

}
