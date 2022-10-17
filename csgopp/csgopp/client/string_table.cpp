#include "string_table.h"

namespace csgopp::client::string_table
{

StringTable::StringTable(const csgo::message::net::CSVCMsg_CreateStringTable& data)
    : name(data.name())
    , entries(data.num_entries())
    , capacity(data.max_entries())
    , data_fixed(data.user_data_fixed_size())
    , data_size_bits(data.user_data_size_bits())
{}

StringTable::StringTable(std::string&& name, size_t size)
    : name(std::move(name))
    , entries(size)
{}

}
