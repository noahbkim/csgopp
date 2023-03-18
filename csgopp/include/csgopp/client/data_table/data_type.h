#pragma once

#include <object/code.h>
#include <object.h>
#include "../../common/bits.h"
#include "../../common/vector.h"
#include "property.h"

namespace csgopp::client::data_table::data_type
{

using csgopp::client::data_table::property::Property;
using csgopp::common::bits::BitStream;
using csgopp::common::vector::Vector2;
using csgopp::common::vector::Vector3;
using object::code::Declaration;
using object::TrivialValueType;
using object::ArrayType;
using object::Type;

struct DataType : public virtual Type
{
    // We are externally guaranteed the passed property will always have created the type
    virtual void update(char* address, BitStream& stream, const Property* property) const = 0;
};

struct BoolType final : public TrivialValueType<bool>, public DataType
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct UnsignedInt32Type final : public TrivialValueType<uint32_t>, public DataType
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct SignedInt32Type final : public TrivialValueType<int32_t>, public DataType
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct FloatType final : public TrivialValueType<float>, public virtual DataType
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct Vector3Type final : public TrivialValueType<Vector3>, public virtual DataType
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct Vector2Type final : public TrivialValueType<Vector2>, public virtual DataType
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct StringType final : public TrivialValueType<std::string>, public virtual DataType
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct UnsignedInt64Type final : public TrivialValueType<uint64_t>, public virtual DataType
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct SignedInt64Type final : public TrivialValueType<int64_t>, public virtual DataType
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct DataArrayType final : public ArrayType, public virtual DataType
{
    using ArrayType::ArrayType;
    void update(char* address, BitStream& stream, const Property* property) const override;
};

}