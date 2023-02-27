#include "entity.h"
#include "data_table.h"

namespace csgopp::client::entity
{

using csgopp::client::data_table::DataTable;


void EntityType::collect_properties_head(
    const DataTable* cursor,
    size_t cursor_offset,
    const std::shared_ptr<const PropertyNode>& parent,
    absl::flat_hash_set<ExcludeView>& excludes,
    std::vector<EntityDatum>& container
)
{
    // TODO: this doesn't 100% reflect the recursion but I'm pretty sure it's fine
    for (const auto& item : cursor->excludes)
    {
        excludes.emplace(item);
    }

    for (DataTable::Property* property : cursor->properties)
    {
        if (excludes.contains(std::make_pair(cursor->name, property->name)))
        {
            continue;
        }

        const auto* data_property = dynamic_cast<DataTable::DataProperty*>(property);

        if (data_property != nullptr)
        {
            // EntityDatum creation
            container.emplace_back(
                this,
                cursor_offset + property->offset,
                data_property,
                parent
            );
        }
        else
        {
            const auto* data_table_property = dynamic_cast<DataTable::DataTableProperty*>(property);
            OK(data_table_property != nullptr);

            auto child = std::make_shared<PropertyNode>(parent, data_table_property);
            if (data_table_property->collapsible())
            {
                this->collect_properties_head(
                    data_table_property->data_table,
                    cursor_offset + property->offset,
                    child,
                    excludes,
                    container
                );
            }
            else
            {
                this->collect_properties_tail(
                    data_table_property->data_table,
                    cursor_offset + property->offset,
                    child,
                    excludes
                );
            }
        }
    }
}

void EntityType::collect_properties_tail(
    const DataTable* cursor,
    size_t cursor_offset,
    const std::shared_ptr<const PropertyNode>& parent,
    absl::flat_hash_set<ExcludeView>& excludes
)
{
    std::vector<EntityDatum> container;
    this->collect_properties_head(cursor, cursor_offset, parent, excludes, container);
    for (const EntityDatum& absolute : container)
    {
        this->prioritized.emplace_back(absolute);
    }
}

void EntityType::prioritize()
{
    size_t start = 0;
    bool more = true;
    for (size_t priority = 0; priority <= 64 || more; ++priority)
    {
        more = false;
        for (size_t i = start; i < this->prioritized.size(); ++i)
        {
            const DataProperty* property = this->prioritized[i].property;
            if (property->priority == priority || priority == 64 && property->changes_often())
            {
                if (start != i)
                {
                    std::swap(this->prioritized[start], this->prioritized[i]);
                }
                start += 1;
            }
            else if (property->priority > priority)
            {
                more = true;
            }
        }
    }
}

EntityType::EntityType(Builder&& builder, const DataTable* data_table)
    : ObjectType(std::move(builder))
    , data_table(data_table)
{
    this->prioritized.reserve(this->members.size());
    absl::flat_hash_set<ExcludeView> excludes;
    this->collect_properties_tail(data_table, 0, nullptr, excludes);
    this->prioritize();
}

Entity::Entity(const EntityType* type, char* address, Id id, const ServerClass* server_class)
    : Instance<EntityType>(type, address)
    , id(id)
    , server_class(server_class)
{
}

}
