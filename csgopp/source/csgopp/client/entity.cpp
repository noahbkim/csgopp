#include "csgopp/client/entity.h"
#include "csgopp/client/data_table.h"

namespace csgopp::client::entity
{

using csgopp::client::data_table::DataTable;

EntityConstantReference::EntityConstantReference(
    std::shared_ptr<const EntityType> origin,
    std::shared_ptr<const char[]> data,
    std::shared_ptr<const Type> type,
    size_t offset,
    std::shared_ptr<const DataProperty> property,
    std::shared_ptr<const PropertyNode> parent
)
    : ConstantReference(std::move(origin), std::move(data), std::move(type), offset)
    , property(std::move(property))
    , parent(std::move(parent))
{
}

Entity::Entity(std::shared_ptr<const EntityType>&& type, Id id, std::shared_ptr<const ServerClass> server_class)
    : Instance<EntityType>(std::move(type))
    , id(id)
    , server_class(std::move(server_class))
{
}

}
