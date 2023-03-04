#pragma once

#include <nanobind/nanobind.h>
#include <csgopp/common/vector.h>
#include <csgopp/common/object.h>

#include "../view.h"

namespace csgopy::view::object_view
{

using csgopp::common::object::ConstReference;
using csgopp::common::object::Reference;
using csgopp::common::object::ConstReference;
using csgopp::common::object::As;
using csgopp::common::object::Is;
using csgopp::common::object::Accessor;
using csgopp::common::object::Type;
using csgopp::common::object::ValueType;
using csgopp::common::vector::Vector2;
using csgopp::common::vector::Vector3;

class ConstReferenceView : public View
{
public:
    explicit ConstReferenceView(ConstReference const_reference)
        : const_reference(const_reference)
    {
    }

    [[nodiscard]] ConstReferenceView get_name(const std::string& name) const
    {
        ConstReferenceView reference(this->const_reference[name]);
        reference.reference(this);
        return reference;
    }

    [[nodiscard]] ConstReferenceView get_index(size_t index) const
    {
        ConstReferenceView reference(this->const_reference[index]);
        reference.reference(this);
        return reference;
    }

    [[nodiscard]] nanobind::object get() const
    {
        const auto* value_type = dynamic_cast<const ValueType*>(this->const_reference.type);
        if (value_type != nullptr)
        {
            if (value_type->info() == typeid(bool))
            {
                return nanobind::cast(this->const_reference.is<bool>(), nanobind::rv_policy::copy);
            }
            if (value_type->info() == typeid(uint32_t))
            {
                return nanobind::cast(this->const_reference.is<uint32_t>(), nanobind::rv_policy::copy);
            }
            if (value_type->info() == typeid(int32_t))
            {
                return nanobind::cast(this->const_reference.is<int32_t>(), nanobind::rv_policy::copy);
            }
            if (value_type->info() == typeid(float))
            {
                return nanobind::cast(this->const_reference.is<float>(), nanobind::rv_policy::copy);
            }
            if (value_type->info() == typeid(Vector3))
            {
                return nanobind::cast(this->const_reference.is<Vector3>(), nanobind::rv_policy::copy);
            }
            if (value_type->info() == typeid(Vector2))
            {
                return nanobind::cast(this->const_reference.is<Vector2>(), nanobind::rv_policy::copy);
            }
            if (value_type->info() == typeid(std::string))
            {
                return nanobind::cast(this->const_reference.is<std::string>(), nanobind::rv_policy::copy);
            }
            if (value_type->info() == typeid(int64_t))
            {
                return nanobind::cast(this->const_reference.is<int64_t>(), nanobind::rv_policy::copy);
            }
            else
            {
                throw std::domain_error("Unreachable: maybe data type is missing a binding?");
            }
        }
        else
        {
            throw std::domain_error("Member is not a value type!");
        }
    }

private:
    ConstReference const_reference;
};


void bind(nanobind::module_& module);

}
