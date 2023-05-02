#pragma once

#include <typeindex>

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <csgopp/common/vector.h>
#include <objective.h>
#include <objective/view.h>

#include "csgopy/adapter.h"

using csgopp::common::vector::Vector2;
using csgopp::common::vector::Vector3;
using objective::Instance;
using objective::ConstantReference;
using objective::Lens;
using objective::Type;
using objective::ValueType;
using objective::View;
using objective::TypeError;

struct ViewBinding
{
    static nanobind::class_<View> bind(nanobind::module_& module, nanobind::class_<View>& base);
};

template<typename T = Type>
struct TypeAdapter : public Adapter<const T>
{
    using Adapter<const T>::Adapter;

    static nanobind::class_<TypeAdapter<T>> bind(nanobind::module_& module, const char* name)
    {
        return nanobind::class_<TypeAdapter<T>>(module, name)
            ;
    }
};

struct LensBinding
{
    static nanobind::class_<Lens> bind(nanobind::module_& module)
    {
        return nanobind::class_<Lens>(module, "Lens")
            .def("type", [](const Lens* self) { return TypeAdapter<Type>(self->type); })
            .def("offset", [](const Lens* self) { return self->offset; })
            .def("__getitem__", [](const Lens* self, std::string name) { return (*self)[name]; })
            .def("__getitem__", [](const Lens* self, size_t index) { return (*self)[index]; })
            .def("__eq__", &Lens::operator==)
            .def("__le__", &Lens::operator<=)
            .def("__lt__", &Lens::operator<)
            .def("__ge__", &Lens::operator>=)
            .def("__gt__", &Lens::operator>)
            ;
    }
};

template<typename T>
nanobind::object cast(const char* address)
{
    return nanobind::cast(*reinterpret_cast<const T*>(address));
}

struct ConstantReferenceBinding
{
    using Caster = nanobind::object (*)(const char*);
    using CasterMap = absl::flat_hash_map<const std::type_info*, Caster>;
    static CasterMap casters;

    static nanobind::class_<ConstantReference> bind(nanobind::module_& module_, nanobind::class_<Lens>& base);
};

template<typename T>
struct InstanceAdapter : public Adapter<const Instance<const T>>
{
    using Adapter<const Instance<const T>>::Adapter;

    [[nodiscard]] TypeAdapter<const T> type() const { return TypeAdapter<const T>(this->self->type); }
    [[nodiscard]] ConstantReference getitem_name(const std::string& name) const { return this->self->operator[](name); }
    [[nodiscard]] ConstantReference getitem_index(size_t index) const { return this->self->operator[](index); }
//    [[nodiscard]] std::optional<std::vector<std::string>> keys() const {}
//    [[nodiscard]] std::optional<size_t> length() const {}

    static nanobind::class_<InstanceAdapter<T>> bind(nanobind::module_& module, const char* name)
    {
        return nanobind::class_<InstanceAdapter<T>>(module, name)
            .def("type", &InstanceAdapter::type)
            .def("__getitem__", &InstanceAdapter::getitem_name)
            .def("__getitem__", &InstanceAdapter::getitem_index)
            ;
    }
};
