#include "object/code.h"

#define TAB "    "

namespace object::code
{

void Declaration::write(std::ostream& out) const
{
    for (const std::string& annotation: this->annotations)
    {
        out << "// " << annotation << std::endl;
    }
    out << "struct " << this->name;
    if (this->base_name)
    {
        out << " : public " << this->base_name.value();
    }
    out << std::endl << "{" << std::endl;
    for (const Member& declaration: this->members)
    {
        if (declaration.annotations.size() > 1)
        {
            for (const std::string& annotation: declaration.annotations)
            {
                out << TAB "// " << annotation << std::endl;
            }
        }
        out << TAB << declaration.type << " " << declaration.name;
        for (size_t array_size: declaration.array_sizes)
        {
            out << "[" << array_size << "]";
        }
        out << ";";
        if (declaration.annotations.size() == 1)
        {
            out << "  // " << declaration.annotations.front();
        }
        out << std::endl;
    }
    out << "};" << std::endl << std::endl;
    if (!this->aliases.empty())
    {
        for (const std::string& alias: this->aliases)
        {
            out << "using " << alias << " = " << this->name << ";" << std::endl;
        }
        out << std::endl;
    }
}

void Generator::sort()
{
    absl::flat_hash_map<std::string, Declaration> lookup;
    absl::flat_hash_map<std::string, absl::flat_hash_set<const Declaration*>> dependencies;
    absl::flat_hash_map<std::string, absl::flat_hash_set<const Declaration*>> dependents;

    lookup.reserve(this->declarations.size());
    std::vector<Declaration> next;

    for (Declaration& declaration : declarations)
    {
        lookup.emplace(declaration.name, std::move(declaration));
    }
    this->declarations.clear();

    for (auto& [name, structure] : lookup)
    {
        if (structure.dependencies.empty())
        {
            next.emplace_back(std::move(structure));
        }
        else
        {
            for (const std::string& dependency : structure.dependencies)
            {
                dependencies[name].emplace(&lookup.at(dependency));
                dependents[dependency].emplace(&structure);
            }
        }
    }

    // TODO: force to terminate if missing dependencies
    while (!next.empty())
    {
        this->declarations.emplace_back(std::move(next.back()));
        next.pop_back();

        const Declaration& back = this->declarations.back();
        for (const Declaration* dependent : dependents[back.name])
        {
            auto& dependent_dependencies = dependencies[back.name];
            dependent_dependencies.erase(&back);
            if (dependent_dependencies.empty())
            {
                next.emplace_back(std::move(lookup.at(dependent->name)));
            }
        }
    }
}

}