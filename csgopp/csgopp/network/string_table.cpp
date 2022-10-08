#include "string_table.h"

namespace csgopp::network
{

StringTable::StringTable(const csgo::message::net::CSVCMsg_CreateStringTable& data)
    : name(data.name())
{

}

}
