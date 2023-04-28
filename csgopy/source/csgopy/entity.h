#pragma once

#include <csgopp/client/entity.h>
#include <objective.h>
#include "adapter.h"
#include "object.h"
#include "server_class.h"

using csgopp::client::entity::Entity;
using csgopp::client::entity::EntityConstantReference;
using csgopp::client::entity::EntityDatum;
using csgopp::client::entity::EntityLens;
using csgopp::client::entity::EntityType;
using csgopp::client::server_class::ServerClass;
using object::ConstantReference;
using object::Lens;

struct EntityLensBinding
{
    static nanobind::class_<EntityLens> bind(nanobind::module_& module_, nanobind::class_<Lens>& base)
    {
        return {module_, "EntityLens", base};
    }
};

struct EntityConstantReferenceBinding
{
    static nanobind::class_<EntityConstantReference> bind(
        nanobind::module_& module_,
        nanobind::class_<ConstantReference>& base
    )
    {
        return {module_, "EntityConstantReference", base};
    }
};

struct EntityTypeAdapter : public Adapter<const EntityType>
{
    using Adapter::Adapter;

    [[nodiscard]] EntityLens property(size_t index) const
    {
        const EntityDatum& datum = this->self->prioritized.at(index);
        return {
            this->self,
            datum.type,
            datum.offset,
            datum.property,
            datum.parent
        };
    }

    [[nodiscard]] Lens at_name(const std::string& name) const
    {
        return Lens(this->self)[name];
    }

    [[nodiscard]] Lens at_index(size_t index) const
    {
        return Lens(this->self)[index];
    }

    static nanobind::class_<EntityTypeAdapter> bind(nanobind::module_& module_)
    {
        return nanobind::class_<EntityTypeAdapter>(module_, "EntityType")
            .def("property", &EntityTypeAdapter::property)
            .def("__getitem__", &EntityTypeAdapter::at_name)
            .def("__getitem__", &EntityTypeAdapter::at_index)
            ;
    }
};

struct EntityAdapter : public Adapter<const Entity>
{
    using Adapter::Adapter;

    [[nodiscard]] Entity::Id id() const
    {
        return this->self->id;
    }

    [[nodiscard]] EntityTypeAdapter type() const
    {
        return EntityTypeAdapter(this->self->type);
    }

    [[nodiscard]] ServerClassAdapter server_class() const
    {
        return ServerClassAdapter(this->self->server_class);
    }

    [[nodiscard]] ServerClass::Index server_class_index() const
    {
        return this->self->server_class->index;
    }

    [[nodiscard]] EntityConstantReference property(size_t index) const
    {
        return this->self->at(index);
    }

    static nanobind::class_<EntityAdapter> bind(nanobind::module_& module_)
    {
        auto base = InstanceAdapter<EntityType>::bind(module_, "EntityTypeInstance");
        return nanobind::class_<EntityAdapter>(module_, "Entity", base)
            .def_prop_ro("id", &EntityAdapter::id)
            .def_prop_ro("type", &EntityAdapter::type)
            .def_prop_ro("server_class", &EntityAdapter::server_class)
            .def_prop_ro("server_class_index", &EntityAdapter::server_class_index)
            .def_prop_ro("property", &EntityAdapter::property)
            ;
    }
};
