#pragma once

#include <string>
#include <vector>
#include <optional>
#include <ostream>
#include <absl/container/flat_hash_set.h>
#include <absl/container/flat_hash_map.h>

namespace csgopp::common::code
{

using Dependencies = absl::flat_hash_set<std::string>;

struct Declaration
{
    std::string type;
    std::string name;
    std::vector<size_t> array_sizes;
    std::vector<std::string> annotations;

    explicit Declaration(std::string name) : name(std::move(name)) {}
};

struct Definition
{
    std::string name;
    std::optional<std::string> base_name;
    std::vector<Declaration> members;
    std::vector<std::string> annotations;

    Definition() = default;
    explicit Definition(std::string name) : name(std::move(name)) {}
    Declaration& append(std::string declaration_name);

    void write(std::ostream& out) const;
};

template<typename T>
struct Cursor
{
    T& target;
    Dependencies& dependencies;

    Cursor(T& target, Dependencies& dependencies)
        : target(target)
        , dependencies(dependencies)
    {}

    template<typename U>
    Cursor<U> into(U& u)
    {
        return Cursor<U>(u, this->dependencies);
    }
};

struct Generator
{
    std::vector<Definition> definitions;

    Cursor<Definition> append(std::string name);

    void write(std::ostream& out);

private:
    absl::flat_hash_map<std::string, absl::flat_hash_set<std::string>> dependencies;
    absl::flat_hash_map<std::string, absl::flat_hash_set<std::string>> dependents;

    void sort();
};

}
