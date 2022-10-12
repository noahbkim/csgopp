#include "string_table.h"

namespace csgopp::network
{

StringTable::StringTable(const std::string& name, size_t entry_count)
    : name(name)
    , entries(entry_count) {}

StringTable::StringTable(std::string&& name, size_t entry_count)
    : name(std::move(name))
    , entries(entry_count) {}

}
