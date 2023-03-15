#include "server_class.h"

namespace csgopp::client::server_class
{

std::shared_ptr<const EntityType> ServerClass::type() const
{
    return this->data_table->type();
}

}