#pragma once

#include <absl/container/flat_hash_map.h>
#include <memory>
#include <string_view>
#include <utility>
#include <vector>

#include "../common/bits.h"
#include "../common/database.h"
#include "../common/vector.h"
#include "data_table.h"
#include "data_table/data_property.h"
#include "data_table/data_table_property.h"
#include <object/code.h>
#include <object/object.h>

namespace csgopp::client::entity
{

using csgopp::client::data_table::data_property::DataProperty;
using csgopp::client::data_table::data_table_property::DataTableProperty;
using csgopp::client::data_table::data_type::DataType;
using csgopp::client::data_table::DataTable;
using csgopp::client::data_table::property::Property;
using csgopp::client::server_class::ServerClass;
using csgopp::common::database::Database;
using object::Accessor;
using object::Instance;
using object::ObjectType;
using object::Type;

// Allows us to get the qualified name of a property
struct PropertyNode
{
    std::shared_ptr<const struct PropertyNode> parent;
    std::shared_ptr<const DataTableProperty> property;

    PropertyNode() = default;

    PropertyNode(std::shared_ptr<const struct PropertyNode> parent, std::shared_ptr<const DataTableProperty> property)
        : parent(std::move(parent))
        , property(std::move(property))
    {}
};

// Maps properties and types
struct EntityDatum : public Accessor
{
    // TODO: These can be raw pointers because they live as long as the type
    std::shared_ptr<const DataType> type;
    std::shared_ptr<const DataProperty> property;

    // Accessing the canonical name of this offset
    std::shared_ptr<const PropertyNode> parent;

    EntityDatum() = default;
    EntityDatum(const EntityDatum& other) = default;
    EntityDatum(
        std::shared_ptr<const Type> origin,  // TODO: limitation: can't be EntityType because of casting
        std::shared_ptr<const DataType> type,
        std::shared_ptr<const DataProperty> property,
        size_t offset,
        std::shared_ptr<const PropertyNode> parent
    )
        : Accessor(std::move(origin), type, offset)
        , type(std::move(type))
        , property(std::move(property))
        , parent(std::move(parent))
    {
    }

    [[nodiscard]] std::string qualified_name() const
    {
        std::string result(this->property->name);
        const PropertyNode* cursor = this->parent.get();
        while (cursor != nullptr)
        {
            result.insert(0, ".");
            result.insert(0, cursor->property->name);
            cursor = cursor->parent.get();
        }

        return result;
    }
};

// TODO: It would be nice to make these a single allocation
using ExcludeView = std::pair<std::string_view, std::string_view>;

struct EntityType final : public ObjectType
{
    /// Flattened, reordered members used for updates
    std::vector<EntityDatum> prioritized;

    using ObjectType::ObjectType;
};

struct Entity final : public Instance<EntityType>
{
    using Id = uint32_t;

    Id id;
    std::shared_ptr<const ServerClass> server_class;

    Entity(std::shared_ptr<const EntityType>&& type, Id id, std::shared_ptr<const ServerClass> server_class);
};

using EntityDatabase = Database<Entity>;

}
