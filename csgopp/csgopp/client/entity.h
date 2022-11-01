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

}

namespace csgopp::client::entity
{

using csgopp::common::vector::Vector3;
using csgopp::common::vector::Vector2;
using csgopp::common::database::Delete;
using csgopp::common::database::Database;
using csgopp::common::database::NameTableMixin;
using csgopp::common::object::Type;
using csgopp::common::object::ObjectType;
using csgopp::common::object::ValueType;
using csgopp::common::object::Instance;
using csgopp::common::object::DefaultValueType;
using csgopp::common::bits::BitStream;
using csgopp::common::code::Declaration;
using csgopp::common::code::Dependencies;
using csgopp::common::code::Cursor;
using csgopp::client::data_table::DataTable;
using csgopp::client::data_table::Property;

// Thinner than accessor; we know what we're doing
enum struct Precision
{
    Normal,
    Low,
    Integral,
};

enum struct Compression
{
    Fixed,
    Variable,
};

enum struct Serialization
{
    Native,
    Optimized,
};

struct PropertyType : public virtual Type
{
    virtual void update(char* address, BitStream& stream) const = 0;
};

struct Offset
{
    const PropertyType* type{nullptr};
    size_t offset{0};
    size_t priority{0};

    Offset() = default;
    Offset(const PropertyType* type, size_t offset);

    [[nodiscard]] Offset from(size_t parent) const;
};

template<Compression C>
struct UnsignedInt32Type final : public virtual DefaultValueType<uint32_t>, public virtual PropertyType
{
    void emit(Cursor<Declaration> cursor) const override;
    void update(char* address, BitStream& stream) const override;
};

template<Compression C>
struct SignedInt32Type final : public virtual DefaultValueType<int32_t>, public virtual PropertyType
{
    void emit(Cursor<Declaration> cursor) const override;
    void update(char* address, BitStream& stream) const override;
};

template<Serialization S>
struct FloatType final : public virtual DefaultValueType<float>, public virtual PropertyType
{
    void emit(Cursor<Declaration> cursor) const override;
    void update(char* address, BitStream& stream) const override;
};

struct BitCoordinateFloatType final : public virtual DefaultValueType<float>, public virtual PropertyType
{
    void emit(Cursor<Declaration> cursor) const override;
    void update(char* address, BitStream& stream) const override;
};

template<Precision P>
struct MultiplayerBitCoordinateFloatType final : public virtual DefaultValueType<float>, public virtual PropertyType
{
    void emit(Cursor<Declaration> cursor) const override;
    void update(char* address, BitStream& stream) const override;
};

struct BitNormalFloatType final : public virtual DefaultValueType<float>, public virtual PropertyType
{
    void emit(Cursor<Declaration> cursor) const override;
    void update(char* address, BitStream& stream) const override;
};

template<Precision P>
struct BitCellCoordinateFloatType final : public virtual DefaultValueType<float>, public virtual PropertyType
{
    void emit(Cursor<Declaration> cursor) const override;
    void update(char* address, BitStream& stream) const override;
};

struct Vector3Type final : public virtual DefaultValueType<Vector3>, public virtual PropertyType
{
    void emit(Cursor<Declaration> cursor) const override;
    void update(char* address, BitStream& stream) const override;
};

struct Vector2Type final : public virtual DefaultValueType<Vector2>, public virtual PropertyType
{
    void emit(Cursor<Declaration> cursor) const override;
    void update(char* address, BitStream& stream) const override;
};

struct StringType final : public virtual DefaultValueType<std::string>, public virtual PropertyType
{
    void emit(Cursor<Declaration> cursor) const override;
    void update(char* address, BitStream& stream) const override;
};

template<Compression C>
struct UnsignedInt64Type final : public virtual DefaultValueType<uint64_t>, public virtual PropertyType
{
    void emit(Cursor<Declaration> cursor) const override;
    void update(char* address, BitStream& stream) const override;
};

template<Compression C>
struct SignedInt64Type final : public virtual DefaultValueType<int64_t>, public virtual PropertyType
{
    void emit(Cursor<Declaration> cursor) const override;
    void update(char* address, BitStream& stream) const override;
};

struct ArrayType final : public virtual csgopp::common::object::ArrayType, public virtual PropertyType
{
    using csgopp::common::object::ArrayType::ArrayType;
    void update(char* address, BitStream& stream) const override{}
};

struct EntityType final : public virtual ObjectType, public virtual PropertyType
{
    /// Fine
    const DataTable* data_table;

    /// Flattened, reordered members used for updates
    std::vector<std::pair<Offset, std::string>> prioritized;

    EntityType(Builder&& builder, const DataTable* data_table);

    void update(char* address, BitStream& stream) const override;
};

using Entity = Instance<EntityType>;
using EntityDatabase = Database<Entity, Delete<Entity>>;

template<Compression C>
void UnsignedInt32Type<C>::update(char* address, BitStream& stream) const
{

}

template<Compression C>
void UnsignedInt32Type<C>::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "uint32_t";
}

template<Compression C>
void SignedInt32Type<C>::update(char* address, BitStream& stream) const
{

}

template<Compression C>
void SignedInt32Type<C>::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "int32_t";
}

template<Serialization S>
void FloatType<S>::update(char* address, BitStream& stream) const
{

}

template<Serialization S>
void FloatType<S>::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "float";
}

template<Precision P>
void MultiplayerBitCoordinateFloatType<P>::update(char* address, BitStream& stream) const
{

}

template<Precision P>
void MultiplayerBitCoordinateFloatType<P>::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "float";
}

template<Precision P>
void BitCellCoordinateFloatType<P>::update(char* address, BitStream& stream) const
{

}

template<Precision P>
void BitCellCoordinateFloatType<P>::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "float";
}

template<Compression C>
void UnsignedInt64Type<C>::update(char* address, BitStream& stream) const
{

}

template<Compression C>
void UnsignedInt64Type<C>::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "uint64_t";
}

template<Compression C>
void SignedInt64Type<C>::update(char* address, BitStream& stream) const
{

}

template<Compression C>
void SignedInt64Type<C>::emit(Cursor<Declaration> cursor) const
{
    cursor.target.type = "int64_t";
}

}
