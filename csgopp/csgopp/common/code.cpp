#include "code.h"

#define TAB "    "

namespace csgopp::common::code
{

void Definition::write(std::ostream& out) const
{
    for (const std::string& annotation : this->annotations)
    {
        out << "// " << annotation << std::endl;
    }
    out << "struct " << this->name;
    if (this->base_name)
    {
        out << " : public " << this->base_name.value();
    }
    out << std::endl << "{" << std::endl;
    for (const Declaration& declaration : this->members)
    {
        if (declaration.annotations.size() > 1)
        {
            for (const std::string& annotation : declaration.annotations)
            {
                out << TAB "// " << annotation << std::endl;
            }
        }
        out << TAB << declaration.type << " " << declaration.name;
        for (size_t array_size : declaration.array_sizes)
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
        for (const std::string& alias : this->aliases)
        {
            out << "using " << alias << " = " << this->name << ";" << std::endl;
        }
        out << std::endl;
    }
}

Declaration& Definition::append(std::string declaration_name)
{
    return this->members.emplace_back(std::move(declaration_name));
}

Cursor<Definition> Generator::append(std::string definition_name)
{
    Definition& definition = this->definitions.emplace_back(std::move(definition_name));
    return Cursor(definition, this->dependencies[definition.name]);
}

void Generator::write(std::ostream& out)
{
    this->sort();
    for (auto& definition : this->definitions)
    {
        definition.write(out);
    }
}

void Generator::sort()
{
    absl::flat_hash_map<std::string, Definition> lookup;
    lookup.reserve(this->definitions.size());
    std::vector<Definition> next;

    for (Definition& definition : definitions)
    {
        lookup.emplace(definition.name, std::move(definition));
    }
    this->definitions.clear();

    for (auto& [name, structure] : lookup)
    {
        if (this->dependencies.at(name).empty())
        {
            next.emplace_back(std::move(structure));
        }
        else
        {
            for (const std::string& dependency : this->dependencies.at(name))
            {
                this->dependents[dependency].emplace(name);
            }
        }
    }

    // TODO: force to terminate if missing dependencies
    while (!next.empty())
    {
        this->definitions.emplace_back(std::move(next.back()));
        next.pop_back();

        const std::string& name = this->definitions.back().name;
        for (const std::string& dependent : this->dependents[name])
        {
            Dependencies& d = this->dependencies.at(dependent);
            d.erase(name);
            if (d.empty())
            {
                next.emplace_back(std::move(lookup.at(dependent)));
            }
        }
    }
}

}