#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace csgopp::game
{

struct SendTable
{
    struct Property
    {
        using Flags = int32_t;

        int32_t raw_type;
        std::string name;
        Flags flags;
        int32_t count_elements;
        float high_value;
        float low_value;
        int32_t count_bits;
    };

    std::vector <Property> properties;
    std::string name;
};

}
