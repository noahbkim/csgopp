#pragma once

#include <objective.h>
#include <objective/code.h>
#include <objective/magic.h>
#include "../../common/bits.h"
#include "../../common/vector.h"
#include "property.h"

namespace csgopp::client::data_table::data_type
{

using csgopp::client::data_table::property::Property;
using csgopp::common::bits::BitStream;
using csgopp::common::vector::Vector2;
using csgopp::common::vector::Vector3;
using objective::code::Declaration;
using objective::TrivialValueType;
using objective::ArrayType;
using objective::WrapperType;
using objective::Type;

struct DataType : public Type
{
    // We are externally guaranteed the passed property will always have created the type
    virtual void update(char* address, BitStream& stream, const Property* property) const = 0;
};

struct BoolType final : public WrapperType<TrivialValueType<bool>, DataType>
{
    using WrapperType::WrapperType;

    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct UnsignedInt32Type final : public WrapperType<TrivialValueType<uint32_t>, DataType>
{
    using WrapperType::WrapperType;

    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct SignedInt32Type final : public WrapperType<TrivialValueType<int32_t>, DataType>
{
    using WrapperType::WrapperType;

    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct FloatType final : public WrapperType<TrivialValueType<float>, DataType>
{
    using WrapperType::WrapperType;

    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct Vector3Type final : public WrapperType<TrivialValueType<Vector3>, DataType>
{
    using WrapperType::WrapperType;

    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct Vector2Type final : public WrapperType<TrivialValueType<Vector2>, DataType>
{
    using WrapperType::WrapperType;

    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct StringType final : public WrapperType<TrivialValueType<std::string>, DataType>
{
    using WrapperType::WrapperType;

    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct UnsignedInt64Type final : public WrapperType<TrivialValueType<uint64_t>, DataType>
{
    using WrapperType::WrapperType;

    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct SignedInt64Type final : public WrapperType<TrivialValueType<int64_t>, DataType>
{
    using WrapperType::WrapperType;

    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, BitStream& stream, const Property* property) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct DataArrayType final : public WrapperType<ArrayType, DataType>
{
    using WrapperType::WrapperType;
    void update(char* address, BitStream& stream, const Property* property) const override;
};

}