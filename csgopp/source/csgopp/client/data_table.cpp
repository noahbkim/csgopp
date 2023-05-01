#include "csgopp/client/data_table.h"
#include "csgopp/client/entity.h"
#include "csgopp/client/server_class.h"
#include "csgopp/common/control.h"

namespace csgopp::client::data_table
{

using csgopp::client::entity::EntityType;
using csgopp::client::entity::EntityDatum;
using csgopp::client::entity::ExcludeView;
using csgopp::client::entity::PropertyNode;
using csgopp::client::data_table::data_property::DataProperty;
using csgopp::common::control::concatenate;
using csgopp::error::GameError;
using objective::make_shared_static;
using objective::Type;

DataTable::DataTable(const CSVCMsg_SendTable& data)
    : name(data.net_table_name())
    , properties(data.props_size())
{
}

void collect_properties_tail(
    std::shared_ptr<EntityType>& entity_type,
    const DataTable* cursor,
    size_t cursor_offset,
    const std::shared_ptr<const PropertyNode>& parent,
    absl::flat_hash_set<ExcludeView>& excludes
);

void collect_properties_head(
    std::shared_ptr<EntityType>& entity_type,
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

    for (const std::shared_ptr<DataTable::Property>& property : cursor->properties)
    {
        if (excludes.contains(std::make_pair(cursor->name, property->name)))
        {
            continue;
        }

        auto data_property = std::dynamic_pointer_cast<const DataTable::DataProperty>(property);

        if (data_property != nullptr)
        {
            // EntityDatum creation
            container.emplace_back(
                data_property->type(),
                data_property,
                cursor_offset + property->offset,
                parent
            );
        }
        else
        {
            auto data_table_property = std::dynamic_pointer_cast<const DataTable::DataTableProperty>(property);
            OK(data_table_property != nullptr);

            auto child = std::make_shared<PropertyNode>(parent, data_table_property);
            if (data_table_property->collapsible())
            {
                collect_properties_head(
                    entity_type,
                    data_table_property->data_table.get(),
                    cursor_offset + property->offset,
                    child,
                    excludes,
                    container
                );
            }
            else
            {
                collect_properties_tail(
                    entity_type,
                    data_table_property->data_table.get(),
                    cursor_offset + property->offset,
                    child,
                    excludes
                );
            }
        }
    }
}

void collect_properties_tail(
    std::shared_ptr<EntityType>& entity_type,
    const DataTable* cursor,
    size_t cursor_offset,
    const std::shared_ptr<const PropertyNode>& parent,
    absl::flat_hash_set<ExcludeView>& excludes
)
{
    std::vector<EntityDatum> container;
    collect_properties_head(entity_type, cursor, cursor_offset, parent, excludes, container);
    for (const EntityDatum& absolute : container)
    {
        entity_type->prioritized.emplace_back(absolute);
    }
}

void prioritize(std::vector<EntityDatum>& prioritized)
{
    size_t start = 0;
    bool more = true;
    for (size_t priority = 0; priority <= 64 || more; ++priority)
    {
        more = false;
        for (size_t i = start; i < prioritized.size(); ++i)
        {
            const std::shared_ptr<const DataProperty>& property = prioritized[i].property;
            if (property->priority == priority || priority == 64 && property->changes_often())
            {
                if (start != i)
                {
                    std::swap(prioritized[start], prioritized[i]);
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

std::shared_ptr<const EntityType> DataTable::construct_type()
{
    if (this->_type == nullptr)
    {
        std::shared_ptr<const EntityType> base;
        EntityType::Builder builder;
        if (this->server_class.lock() && this->server_class.lock()->base_class != nullptr)
        {
            // TODO make this better
            base = this->server_class.lock()->base_class->data_table->construct_type();
            builder = EntityType::Builder(base);
        }

        builder.name = this->name;
        builder.metadata = this;
        for (const std::shared_ptr<DataTable::Property>& property : this->properties)
        {
            property->build(builder);
        }

        auto entity_type = std::make_shared<EntityType>(std::move(builder));

        // Create prioritized
        entity_type->prioritized.reserve(entity_type->members.size());
        absl::flat_hash_set<ExcludeView> accumulated_excludes;
        collect_properties_tail(entity_type, this, 0, nullptr, accumulated_excludes);
        prioritize(entity_type->prioritized);

        // Assign
        this->_type = entity_type;
    }

    return this->_type;
}

std::shared_ptr<const EntityType> DataTable::type() const
{
    return this->_type;
}

std::shared_ptr<const ArrayType> DataTable::construct_array_type()
{
    if (this->_array_type == nullptr)
    {
        std::shared_ptr<const Type> element_type = this->properties.at(0)->construct_type();

        // We have to manually set offsets since we're not materializing
        for (size_t i = 0; i < this->properties.size(); ++i)
        {
            // TODO: this should probably be const, offset should be set elsewhere
            this->properties.at(i)->offset = i * element_type->size();
        }

        size_t array_size = this->properties.size();
        this->_array_type = std::make_shared<ArrayType>(element_type, array_size);
    }

    return this->_array_type;
}


void DataTable::attach(Declaration& declaration)
{
    if (this->server_class.lock())
    {
        declaration.aliases.emplace_back(this->server_class.lock()->name);
    }
}

/// Left pad with zeros to 3 characters
bool is_array_index(std::string_view name, size_t index)
{
    if (name.size() < 3)
    {
        return false;
    }

    size_t i{0};
    for (; i < 3; ++i)
    {
        if (name[name.size() - 1 - i] != '0' + index % 10)
        {
            return false;
        }

        index /= 10;
    }

    for (; i < name.size(); ++i)
    {
        if (index == 0 || name[name.size() - 1 - i] != '0' + index % 10)
        {
            return false;
        }

        index /= 10;
    }

    return true;
}

}
