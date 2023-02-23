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

// Maps properties and types
struct EntityDatum : public Accessor
{
    // TODO: we can reduce one memory lookup by somehow getting this type to be DataType* type
    const DataProperty* property{nullptr};
    const DataType* data_type{nullptr};

    EntityDatum() = default;

    EntityDatum(
        const struct Type* origin,
        const DataType* data_type,
        size_t offset,
        const DataProperty* property
    )
        : Accessor(origin, data_type, offset)
        , property(property)
        , data_type(data_type)
    {
        std::shared_ptr<const DataType> dt = property->data_type();
        this->data_type = dt.get();
//        OK(this->data_type == this->type);
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
        absl::flat_hash_set<ExcludeView>& excludes,
        std::vector<EntityDatum>& container
    );

    void collect_properties_tail(
        const DataTable* cursor,
        size_t cursor_offset,
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
