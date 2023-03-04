#pragma once

#include <stdexcept>

#include <nanobind/nanobind.h>
#include <csgopp/client/entity.h>

#include "../view.h"
#include "object_view.h"

namespace csgopy::view::entity_view
{

using csgopp::client::entity::Entity;
using csgopp::common::object::ConstReference;
using csgopy::view::object_view::ConstReferenceView;

class EntityView : public View
{
public:
    explicit EntityView(const Entity* entity)
        : entity(entity)
    {
    }

    [[nodiscard]] Entity::Id id() const
    {
        this->assert_valid();
        return this->entity->id;
    }

    [[nodiscard]] ConstReferenceView get_name(const std::string& name) const
    {
        ConstReferenceView reference((*this->entity)[name]);
        reference.reference(this);
        return reference;
    }

    [[nodiscard]] ConstReferenceView get_index(size_t index) const
    {
        ConstReferenceView reference((*this->entity)[index]);
        reference.reference(this);
        return reference;
    }

    [[nodiscard]] ConstReference get(const std::string& name) const
    {
        throw std::domain_error("Member is not a value type!");
    }

private:
    const Entity* entity{nullptr};
};

void bind(nanobind::module_& module);

}
