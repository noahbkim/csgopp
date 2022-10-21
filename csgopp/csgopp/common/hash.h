#pragma once

#include <vector>
#include <string>
#include <iostream>

namespace csgopp::common::hash
{

void print(const std::vector<bool>& v)
{
    for (bool item : v)
    {
        std::cout << item << ", ";
    }
    std::cout << std::endl;
}

struct Hash
{
    uint32_t k;
    uint32_t p;

    [[nodiscard]] size_t compute(std::string_view key) const
    {
        return (this->k * std::hash<std::string_view>{}(key)) % this->p;
    }
};

template<typename Value>
size_t test(Hash hash, std::vector<std::pair<std::string, Value>> pairs)
{
    size_t collisions = 0;
    std::vector<bool> coverage(pairs.size(), false);
    for (std::pair<std::string, Value> pair : pairs)
    {
        size_t index = std::hash<std::string_view>{}(pair.first) % coverage.size();
        while (coverage.at(index))
        {
            index = (index + 1) % coverage.size();
        }
        coverage.at(index) = true;
        collisions += 1;
    }
    return collisions;
}

template<typename Value>
class Lookup
{
public:
    class Builder
    {
    public:
        void add(const std::string& key, Value value)
        {
            this->contents.emplace_back(key, value);
        }

        bool build() const
        {
            for (uint32_t k = 1; k < 10; ++k)
            {
                uint32_t p = this->contents.size();
                Hash hash{k, p};
                std::cout << "[k: " << k << ", p: " << p << "] " << test(hash, this->contents) << std::endl;
            }
            return false;
        }

    private:
        std::vector<std::pair<std::string, Value>> contents;
    };

private:
    size_t size;
};

}
