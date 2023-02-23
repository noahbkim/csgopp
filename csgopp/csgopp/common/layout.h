#pragma once

#include <string>
#include <vector>
#include <ostream>

namespace csgopp::common::layout
{

struct Cursor
{
    std::ostream& out;
    size_t absolute;
    size_t level;

    explicit Cursor(std::ostream& out, size_t absolute = 0, size_t level = 0)
        : out(out)
        , absolute(absolute)
        , level(level)
    {
    }

    Cursor indent(size_t relative)
    {
        return Cursor(this->out, this->absolute + relative, this->level + 1);
    }

    void write(std::string_view name, size_t relative)
    {
        this->out << std::endl;

        for (size_t i = 0; i <= this->level; ++i)
        {
            this->out << "  ";
        }

        this->out << name << " " << this->absolute + relative;
    }

    void note(std::string_view note)
    {
        this->out << " (" << note << ")";
    }
};

}
