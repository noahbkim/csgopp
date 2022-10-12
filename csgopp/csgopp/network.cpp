#include "network.h"

namespace csgopp::network
{

void Network::publish_data_table(DataTable* data_table)
{
    this->_data_tables.emplace_back(data_table);
    this->_data_tables_by_name.emplace(data_table->name, data_table);
}

void Network::publish_server_class(ServerClass* server_class)
{
    this->_server_classes.emplace_back(server_class);
    this->_server_classes_by_name.emplace(server_class->name, server_class);
    this->_server_classes_by_id.emplace(server_class->id, server_class);
}

void Network::publish_string_table(StringTable* string_table)
{
    this->_string_tables.emplace_back(string_table);
    this->_string_tables_by_name.emplace(string_table->name, string_table);
}

}
