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
using csgopp::client::data_table::DataTable;
using csgopp::client::server_class::ServerClass;

using csgopp::client::data_table::data_type::DataType;
using csgopp::client::data_table::data_property::DataProperty;
using csgopp::client::data_table::data_table_property::DataTableProperty;

struct Offset
{
    size_t offset{0};

    Offset() = default;
    explicit Offset(size_t offset)
        : offset(offset)
    {}
};

struct DataTableOffset : Offset
{
    std::shared_ptr<const DataTableOffset> parent;
    const DataTableProperty* property{nullptr};
    const Type* type{nullptr};

    DataTableOffset() = default;
    DataTableOffset(size_t offset, std::shared_ptr<const DataTableOffset> parent, const DataTableProperty* property)
        : Offset(offset)
        , parent(std::move(parent))
        , property(property)
        , type(property->type().get())
    {}
};

struct DataOffset : public Offset
{
    std::shared_ptr<const DataTableOffset> parent;
    const DataProperty* property{nullptr};
    const DataType* type{nullptr};

    DataOffset() = default;
    DataOffset(size_t offset, std::shared_ptr<const DataTableOffset> parent, const DataProperty* property)
        : Offset(offset)
        , parent(std::move(parent))
        , property(property)
        , type(property->data_type().get())
    {}
};

// TODO: It would be nice to make these a single allocation
using ExcludeView = std::pair<std::string_view, std::string_view>;

struct EntityType final : public ObjectType
{
    /// Fine
    const DataTable* data_table;

    /// Flattened, reordered members used for updates
    std::vector<DataOffset> prioritized;

    EntityType(Builder&& builder, const DataTable* data_table);

    void collect_properties_head(
        const DataTable* cursor,
        const std::shared_ptr<const DataTableOffset>& cursor_parent,
        size_t cursor_offset,
        absl::flat_hash_set<ExcludeView>& excludes,
        std::vector<DataOffset>& container);

    void collect_properties_tail(
        const DataTable* cursor,
        const std::shared_ptr<const DataTableOffset>& cursor_parent,
        size_t cursor_offset,
        absl::flat_hash_set<ExcludeView>& excludes);

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
