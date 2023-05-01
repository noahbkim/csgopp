#pragma once

#pragma once

#include <string>
#include <utility>
#include <vector>
#include <optional>
#include <ostream>
#include <absl/container/flat_hash_set.h>
#include <absl/container/flat_hash_map.h>

namespace objective::code
{

struct Declaration
{
    struct Member
    {
        std::string type;
        std::string name;
        std::vector<size_t> array_sizes;
        std::vector<std::string> annotations;

        Member() = default;
        explicit Member(std::string name) : name(std::move(name)) {}
    };

    std::string name;
    std::optional<std::string> base_name;
    std::vector<Member> members;
    std::vector<std::string> annotations;
    std::vector<std::string> aliases;
    absl::flat_hash_set<std::string> dependencies;

    Member& append(const std::string& member_name)
    {
        return this->members.emplace_back(member_name);
    }

    void write(std::ostream& out) const;
};

template<typename... Args>
struct Metadata
{
    virtual void attach(Args...) {}
};

void sort(std::vector<Declaration>& declarations);

struct Generator
{
    std::vector<Declaration> declarations;

    Declaration& append()
    {
        return this->declarations.emplace_back();
    }

    void sort();

    void write(std::ostream& out, bool sort = true)
    {
        for (auto& definition: this->declarations)
        {
            definition.write(out);
        }
    }
};

}

