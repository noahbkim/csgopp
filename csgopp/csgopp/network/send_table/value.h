#pragma once

#include <cstdint>

#include "../../common/vector.h"
#include "property.h"

namespace csgopp::network::send_table
{

using csgopp::common::vector::Vector2;
using csgopp::common::vector::Vector3;

struct Value
{
    virtual ~Value() = default;

    [[nodiscard]] virtual Property* origin() const = 0;
//        virtual void deserialize(CodedInputStream& stream) = 0;
};

struct Int32Value final : public Value
{
    Int32Property* property;
    int32_t value;

    [[nodiscard]] Property* origin() const override { return this->property; }
//        void update(CodedInputStream& stream) override;
};


struct FloatValue final : public Value
{
    FloatProperty* property;
    float value;

    [[nodiscard]] Property* origin() const override { return this->property; }
//        void update(CodedInputStream& stream) override;
};


struct Vector3Value final : public Value
{
    Vector3Property* property;
    Vector3 value;

    [[nodiscard]] Property* origin() const override { return this->property; }
//        void update(CodedInputStream& stream) override;
};


struct Vector2Value final : public Value
{
    Vector2Property* property;
    Vector2 value;

    [[nodiscard]] Property* origin() const override { return this->property; }
//        void update(CodedInputStream& stream) override;
};


struct StringValue final : public Value
{
    StringProperty* property;
    std::string value;

    [[nodiscard]] Property* origin() const override { return this->property; }
//        void update(CodedInputStream& stream) override;
};


struct ArrayValue final : public Value
{
    Vector2Property* property;
    std::vector<Property*> value;

    [[nodiscard]] Property* origin() const override { return this->property; }
//        void update(CodedInputStream& stream) override;
};


struct DataTableValue final : public Value
{
    DataTableProperty* property;
    // TODO: ??

    [[nodiscard]] Property* origin() const override { return this->property; }
//        void update(CodedInputStream& stream) override;
};


struct Int64Value final : public Value
{
    Int64Property* property;
    int64_t value;

    [[nodiscard]] Property* origin() const override { return this->property; }
//        void update(CodedInputStream& stream) override;
};

}
