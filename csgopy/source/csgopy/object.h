#pragma once

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <csgopp/common/vector.h>
#include <object.h>
#include <object/view.h>
#include "adapter.h"

using csgopp::common::vector::Vector2;
using csgopp::common::vector::Vector3;
using object::Instance;
using object::ConstantReference;
using object::Lens;
using object::Type;
using object::ValueType;
using object::View;
using object::TypeError;

struct ViewAdapter
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
            .def("__repr__", [name](const TypeAdapter<T>* adapter) { return std::string(name) + "<" + adapter->self->represent() + ">"; })
            ;
    }
};

struct LensAdapter
{
    static nanobind::class_<Lens> bind(nanobind::module_& module)
    {
        return nanobind::class_<Lens>(module, "Lens")
            .def("type", [](const Lens* self) { return TypeAdapter<Type>(self->type); })
            .def("offset", [](const Lens* self) { return self->offset; })
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

struct ConstantReferenceAdapter
{
    using Caster = nanobind::object (*)(const char*);
    using CasterMap = absl::flat_hash_map<const std::type_info*, Caster>;
    static CasterMap casters;

    static nanobind::class_<ConstantReference> bind(nanobind::module_& module_, nanobind::class_<Lens>& base)
    {
        return nanobind::class_<ConstantReference>(module_, "ConstantReference", base)
            .def("__getitem__", [](const ConstantReference* self, const std::string& name) { return self->operator[](name); })
            .def("__getitem__", [](const ConstantReference* self, size_t index) { return self->operator[](index); })
            .def("type", [](const ConstantReference* self) { return TypeAdapter<Type>(self->type); })
            .def("value", [](const ConstantReference* self)
            {
                auto* value_type = dynamic_cast<const ValueType*>(self->type.get());
                if (value_type != nullptr)
                {
                    Caster caster = ConstantReferenceAdapter::casters[&value_type->info()];
                    return caster(self->data.get());
                }
                else
                {
                    throw TypeError("cast is only available for values!");
                }
            })
            ;
    }
};

template<typename T>
struct InstanceAdapter : public Adapter<const Instance<const T>>
{
    using Adapter<const Instance<const T>>::Adapter;

    [[nodiscard]] TypeAdapter<const T> type() const { return TypeAdapter<const T>(this->self->type); }
    [[nodiscard]] ConstantReference get_name(const std::string& name) const { return this->self->operator[](name); }
    [[nodiscard]] ConstantReference get_index(size_t index) const { return this->self->operator[](index); }

    static nanobind::class_<InstanceAdapter<T>> bind(nanobind::module_& module, const char* name)
    {
        return nanobind::class_<InstanceAdapter<T>>(module, name)
            .def("type", &InstanceAdapter::type)
            .def("__getitem__", &InstanceAdapter::get_name)
            .def("__getitem__", &InstanceAdapter::get_index)
            ;
    }
};
