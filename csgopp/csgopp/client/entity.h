#pragma once

#include <memory>
#include <vector>
#include <string_view>
#include <absl/container/flat_hash_map.h>

#include "../common/vector.h"
#include "../common/database.h"
#include "../common/object.h"
#include "../common/code.h"
#include "../common/bits.h"
#include "data_table.h"
#include "data_table/data_property.h"
#include "data_table/data_table_property.h"

namespace csgopp::client::entity
{

using csgopp::common::database::Delete;
using csgopp::common::database::Database;
using csgopp::common::object::ObjectType;
using csgopp::common::object::Instance;
using csgopp::common::object::Type;
using csgopp::common::object::Accessor;
using csgopp::client::data_table::DataTable;
using csgopp::client::data_table::property::Property;
using csgopp::client::data_table::data_type::DataType;
using csgopp::client::data_table::data_property::DataProperty;
using csgopp::client::data_table::data_table_property::DataTableProperty;
using csgopp::client::server_class::ServerClass;

// Allows us to get the qualified name of a property
struct PropertyNode
{
    std::shared_ptr<const struct PropertyNode> parent;
    const DataTableProperty* property{nullptr};

    PropertyNode() = default;

    PropertyNode(std::shared_ptr<const struct PropertyNode> parent, const DataTableProperty* property)
        : parent(std::move(parent))
        , property(property)
    {}
};

// Maps properties and types
struct EntityDatum : public Accessor
{
    // TODO: we can reduce one memory lookup by somehow getting this type to be DataType* type
    const DataProperty* property{nullptr};

    // Caching this dramatically improves runtime
    const DataType* data_type{nullptr};

    // Accessing the canonical name of this offset
    std::shared_ptr<const PropertyNode> parent;

    EntityDatum() = default;

    EntityDatum(
        const struct Type* origin,
        size_t offset,
        const DataProperty* property,
        std::shared_ptr<const PropertyNode> parent
    )
        : Accessor(origin, property->type(), offset)
        , property(property)
        , data_type(property->type())
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
    /// Fine
    const DataTable* data_table;

    /// Flattened, reordered members used for updates
    std::vector<EntityDatum> prioritized;

    EntityType(Builder&& builder, const DataTable* data_table);

    void collect_properties_head(
        const DataTable* cursor,
        size_t cursor_offset,
        const std::shared_ptr<const PropertyNode>& parent,
        absl::flat_hash_set<ExcludeView>& excludes,
        std::vector<EntityDatum>& container
    );

    void collect_properties_tail(
        const DataTable* cursor,
        size_t cursor_offset,
        const std::shared_ptr<const PropertyNode>& parent,
        absl::flat_hash_set<ExcludeView>& excludes
    );

    void prioritize();
};

struct Entity final : public Instance<EntityType>
{
    using Id = uint32_t;

    Id id;
    const ServerClass* server_class;

    Entity(const EntityType* type, char* address, Id id, const ServerClass* server_class);
};

using EntityDatabase = Database<Entity, Delete<Entity>>;

}
