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

namespace csgopp::client::data_table
{

struct Property;
struct DataTable;
struct DataTableProperty;

}

namespace csgopp::client::server_class
{

struct ServerClass;

}

namespace csgopp::client::entity
{

using csgopp::common::vector::Vector3;
using csgopp::common::vector::Vector2;
using csgopp::common::database::Delete;
using csgopp::common::database::Database;
using csgopp::common::database::NameTableMixin;
using csgopp::common::object::Type;
using csgopp::common::object::ValueType;
using csgopp::common::object::ArrayType;
using csgopp::common::object::ObjectType;
using csgopp::common::object::Instance;
using csgopp::common::object::DefaultValueType;
using csgopp::common::bits::BitStream;
using csgopp::common::code::Declaration;
using csgopp::common::code::Dependencies;
using csgopp::common::code::Cursor;
using csgopp::client::data_table::DataTable;
using csgopp::client::data_table::Property;
using csgopp::client::data_table::DataTableProperty;
using csgopp::client::server_class::ServerClass;

enum struct Precision
{
    Normal,
    Low,
};

struct PropertyValueType : public virtual Type
{
    virtual void update(char* address, BitStream& stream, const Property* property) const = 0;
};

struct Offset
{
    const PropertyValueType* type{nullptr};
    const Property* property{nullptr};
    size_t offset{0};

    Offset() = default;
    Offset(const PropertyValueType* type, const Property* property, size_t offset);

    Offset relative(size_t to) const;
};

struct BoolType final : public DefaultValueType<bool>, public PropertyValueType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
    void represent(const char* address, std::ostream& out) const override;
};

struct UnsignedInt32Type final : public DefaultValueType<uint32_t>, public PropertyValueType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
    void represent(const char* address, std::ostream& out) const override;
};

struct SignedInt32Type final : public DefaultValueType<int32_t>, public PropertyValueType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
    void represent(const char* address, std::ostream& out) const override;
};

struct FloatType final : public DefaultValueType<float>, public virtual PropertyValueType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
    void represent(const char* address, std::ostream& out) const override;
};

struct Vector3Type final : public DefaultValueType<Vector3>, public virtual PropertyValueType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
    void represent(const char* address, std::ostream& out) const override;
};

struct Vector2Type final : public DefaultValueType<Vector2>, public virtual PropertyValueType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
    void represent(const char* address, std::ostream& out) const override;
};

struct StringType final : public DefaultValueType<std::string>, public virtual PropertyValueType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
    void represent(const char* address, std::ostream& out) const override;
};

struct UnsignedInt64Type final : public DefaultValueType<uint64_t>, public virtual PropertyValueType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
    void represent(const char* address, std::ostream& out) const override;
};

struct SignedInt64Type final : public DefaultValueType<int64_t>, public virtual PropertyValueType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
    void represent(const char* address, std::ostream& out) const override;
};

struct PropertyArrayType final : public ArrayType, public virtual PropertyValueType
{
    using ArrayType::ArrayType;
    void update(char* address, BitStream& stream, const Property* property) const override;
};

struct EntityOffset : public Offset
{
    /// Maximum property count when flattened is uint16_t, add extra for nesting (though this is unbounded?)
    std::optional<uint32_t> parent;

    EntityOffset() = default;
    EntityOffset(
        const PropertyValueType* type,
        const Property* property,
        size_t offset,
        std::optional<uint32_t> parent);

    static EntityOffset from(
        const Offset& offset,
        size_t parent_offset,
        std::optional<uint32_t> parent);
};

struct EntityType final : public ObjectType
{
    /// Fine
    const DataTable* data_table;

    /// Flattened, reordered members used for updates
    std::vector<Offset> prioritized;

    EntityType(Builder&& builder, const DataTable* data_table, std::vector<Offset>&& prioritized);
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
