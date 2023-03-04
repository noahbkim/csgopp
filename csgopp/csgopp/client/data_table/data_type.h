#pragma once

#include <object/code.h>
#include <object/object.h>
#include "../../common/bits.h"
#include "../../common/vector.h"
#include "property.h"

namespace csgopp::client::data_table::data_type
{

using csgopp::client::data_table::property::Property;
using csgopp::common::bits::BitStream;
using csgopp::common::vector::Vector2;
using csgopp::common::vector::Vector3;
using object::code::Cursor;
using object::code::Declaration;
using object::DefaultValueType;
using object::ArrayType;
using object::Type;

struct DataType : public virtual Type
{
    // We are externally guaranteed the passed property will always have created the type
    virtual void update(char* address, BitStream& stream, const Property* property) const = 0;
};

struct BoolType final : public DefaultValueType<bool>, public DataType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
    void represent(const char* address, std::ostream& out) const override;
};

struct UnsignedInt32Type final : public DefaultValueType<uint32_t>, public DataType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
    void represent(const char* address, std::ostream& out) const override;
};

struct SignedInt32Type final : public DefaultValueType<int32_t>, public DataType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
    void represent(const char* address, std::ostream& out) const override;
};

struct FloatType final : public DefaultValueType<float>, public virtual DataType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
    void represent(const char* address, std::ostream& out) const override;
};

struct Vector3Type final : public DefaultValueType<Vector3>, public virtual DataType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
    void represent(const char* address, std::ostream& out) const override;
};

struct Vector2Type final : public DefaultValueType<Vector2>, public virtual DataType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
    void represent(const char* address, std::ostream& out) const override;
};

struct StringType final : public DefaultValueType<std::string>, public virtual DataType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
    void represent(const char* address, std::ostream& out) const override;
};

struct UnsignedInt64Type final : public DefaultValueType<uint64_t>, public virtual DataType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
    void represent(const char* address, std::ostream& out) const override;
};

struct SignedInt64Type final : public DefaultValueType<int64_t>, public virtual DataType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
    void represent(const char* address, std::ostream& out) const override;
};

struct DataArrayType final : public ArrayType, public virtual DataType
{
    using ArrayType::ArrayType;
    void update(char* address, BitStream& stream, const Property* property) const override;
};

}