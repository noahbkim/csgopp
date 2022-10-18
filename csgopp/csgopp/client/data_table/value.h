#pragma once

#include <cstdint>

#include "../../common/vector.h"
#include "../../common/bits.h"
#include "property.h"

namespace csgopp::client::data_table::value
{

using csgopp::common::vector::Vector2;
using csgopp::common::vector::Vector3;
using csgopp::common::bits::BitStream;
using namespace csgopp::client::data_table::property;

struct Value
{
    using Index = uint16_t;
    using Type = Property::Type;

    Value() = default;
    virtual ~Value() = default;

    [[nodiscard]] virtual Property* origin() const = 0;

    virtual void deserialize(BitStream& stream) = 0;
};

struct Int32Value final : public Value
{
    Int32Property* property;
    int32_t value{};

    explicit Int32Value(Int32Property* property) : property(property) {}

    [[nodiscard]] Property* origin() const override { return this->property; }

    void deserialize(BitStream& stream) override;
};

struct FloatValue final : public Value
{
    FloatProperty* property;
    float value{};

    explicit FloatValue(FloatProperty* property) : property(property) {}

    [[nodiscard]] Property* origin() const override { return this->property; }

    void deserialize(BitStream& stream) override;
};

struct Vector3Value final : public Value
{
    Vector3Property* property;
    Vector3 value{};

    explicit Vector3Value(Vector3Property* property) : property(property) {}

    [[nodiscard]] Property* origin() const override { return this->property; }

    void deserialize(BitStream& stream) override;
};

struct Vector2Value final : public Value
{
    Vector2Property* property;
    Vector2 value{};

    explicit Vector2Value(Vector2Property* property) : property(property) {}

    [[nodiscard]] Property* origin() const override { return this->property; }

    void deserialize(BitStream& stream) override;
};

struct StringValue final : public Value
{
    StringProperty* property;
    std::string value;

    explicit StringValue(StringProperty* property) : property(property) {}

    [[nodiscard]] Property* origin() const override { return this->property; }

    void deserialize(BitStream& stream) override;
};

struct ArrayValue final : public Value
{
    ArrayProperty* property;
    std::vector<Property*> value;

    explicit ArrayValue(ArrayProperty* property) : property(property) {}

    [[nodiscard]] Property* origin() const override { return this->property; }

    void deserialize(BitStream& stream) override;
};

struct DataTableValue final : public Value
{
    DataTableProperty* property;

    explicit DataTableValue(DataTableProperty* property) : property(property) {}

    [[nodiscard]] Property* origin() const override { return this->property; }

    void deserialize(BitStream& stream) override;
};

struct Int64Value final : public Value
{
    Int64Property* property;
    int64_t value{};

    explicit Int64Value(Int64Property* property) : property(property) {}

    [[nodiscard]] Property* origin() const override { return this->property; }

    void deserialize(BitStream& stream) override;
};

}
