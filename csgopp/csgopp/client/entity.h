#pragma once

#include <memory>
#include <vector>
#include <string_view>
#include <absl/container/flat_hash_map.h>

#include "../common/vector.h"
#include "../common/database.h"
#include "../common/object.h"
#include "../common/bits.h"

namespace csgopp::client::data_table
{

struct DataTable;

}

namespace csgopp::client::entity
{

using csgopp::common::vector::Vector3;
using csgopp::common::vector::Vector2;
using csgopp::common::database::Delete;
using csgopp::common::database::Database;
using csgopp::common::database::NameTableMixin;
using csgopp::common::object::ObjectType;
using csgopp::common::object::ValueType;
using csgopp::common::object::Instance;
using csgopp::common::object::DefaultValueType;
using csgopp::common::bits::BitStream;
using csgopp::client::data_table::DataTable;

// Thinner than accessor; we know what we're doing
struct Offset
{
    const struct Type* type;
    size_t offset;
};

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

struct EntityValueType : public ValueType
{
    virtual void update(char* address, BitStream& stream) const = 0;
};

template<Compression C>
struct UnsignedInt32Type final : public DefaultValueType<uint32_t, EntityValueType>
{
    [[nodiscard]] std::string declare(const std::string& name) const override;
    void update(char* address, BitStream& stream) const override;
};

template<Compression C>
struct SignedInt32Type final : public DefaultValueType<int32_t, EntityValueType>
{
    [[nodiscard]] std::string declare(const std::string& name) const override;
    void update(char* address, BitStream& stream) const override;
};

template<Serialization S>
struct FloatType final : public DefaultValueType<float, EntityValueType>
{
    [[nodiscard]] std::string declare(const std::string& name) const override;
    void update(char* address, BitStream& stream) const override;
};

struct BitCoordinateFloatType final : public DefaultValueType<float, EntityValueType>
{
    [[nodiscard]] std::string declare(const std::string& name) const override;
    void update(char* address, BitStream& stream) const override;
};

template<Precision P>
struct MultiplayerBitCoordinateFloatType final : public DefaultValueType<float, EntityValueType>
{
    [[nodiscard]] std::string declare(const std::string& name) const override;
    void update(char* address, BitStream& stream) const override;
};

struct BitNormalFloatType final : public DefaultValueType<float, EntityValueType>
{
    [[nodiscard]] std::string declare(const std::string& name) const override;
    void update(char* address, BitStream& stream) const override;
};

template<Precision P>
struct BitCellCoordinateFloatType final : public DefaultValueType<float, EntityValueType>
{
    [[nodiscard]] std::string declare(const std::string& name) const override;
    void update(char* address, BitStream& stream) const override;
};

struct Vector3Type final : public DefaultValueType<Vector3, EntityValueType>
{
    [[nodiscard]] std::string declare(const std::string& name) const override;
    void update(char* address, BitStream& stream) const override;
};

struct Vector2Type final : public DefaultValueType<Vector2, EntityValueType>
{
    [[nodiscard]] std::string declare(const std::string& name) const override;
    void update(char* address, BitStream& stream) const override;
};

struct StringType final : public DefaultValueType<std::string, EntityValueType>
{
    [[nodiscard]] std::string declare(const std::string& name) const override;
    void update(char* address, BitStream& stream) const override;
};

template<Compression C>
struct UnsignedInt64Type final : public DefaultValueType<uint64_t, EntityValueType>
{
    [[nodiscard]] std::string declare(const std::string& name) const override;
    void update(char* address, BitStream& stream) const override;
};

template<Compression C>
struct SignedInt64Type final : public DefaultValueType<int64_t, EntityValueType>
{
    [[nodiscard]] std::string declare(const std::string& name) const override;
    void update(char* address, BitStream& stream) const override;
};

struct EntityType final : public ObjectType
{
    /// Fine
    const DataTable* data_table;

    /// Flattened, reordered members used for updates
    std::vector<std::shared_ptr<Offset>> prioritized;

    EntityType(Builder&& builder, const DataTable* data_table)
        : ObjectType(std::move(builder))
        , data_table(data_table)
    {}
};

using Entity = Instance<EntityType>;
using EntityDatabase = Database<Entity, Delete<Entity>>;

template<Compression C>
void UnsignedInt32Type<C>::update(char* address, BitStream& stream) const
{

}

template<Compression C>
std::string UnsignedInt32Type<C>::declare(const std::string& name) const
{
    return "uint32_t " + name;
}

template<Compression C>
void SignedInt32Type<C>::update(char* address, BitStream& stream) const
{

}

template<Compression C>
std::string SignedInt32Type<C>::declare(const std::string& name) const
{
    return "int32_t " + name;
}

template<Serialization S>
void FloatType<S>::update(char* address, BitStream& stream) const
{

}

template<Serialization S>
std::string FloatType<S>::declare(const std::string& name) const
{
    return "float " + name;
}

template<Precision P>
void MultiplayerBitCoordinateFloatType<P>::update(char* address, BitStream& stream) const
{

}

template<Precision P>
std::string MultiplayerBitCoordinateFloatType<P>::declare(const std::string& name) const
{
    return "float " + name;
}

template<Precision P>
void BitCellCoordinateFloatType<P>::update(char* address, BitStream& stream) const
{

}

template<Precision P>
std::string BitCellCoordinateFloatType<P>::declare(const std::string& name) const
{
    return "float " + name;
}

template<Compression C>
void UnsignedInt64Type<C>::update(char* address, BitStream& stream) const
{

}

template<Compression C>
std::string UnsignedInt64Type<C>::declare(const std::string& name) const
{
    return "uint64_t " + name;
}

template<Compression C>
void SignedInt64Type<C>::update(char* address, BitStream& stream) const
{

}

template<Compression C>
std::string SignedInt64Type<C>::declare(const std::string& name) const
{
    return "int64_t " + name;
}

}
