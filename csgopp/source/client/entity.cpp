#include "entity.h"
#include "data_table.h"

namespace csgopp::client::entity
{

using csgopp::client::data_table::DataTable;

Entity::Entity(std::shared_ptr<const EntityType>&& type, Id id, std::shared_ptr<const ServerClass> server_class)
    : Instance<EntityType>(std::move(type))
    , id(id)
    , server_class(std::move(server_class))
{
}

}
