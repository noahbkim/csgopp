#include "csgopy/objective/object.h"

nanobind::class_<ConstantReference> ConstantReferenceBinding::bind(nanobind::module_& module_, nanobind::class_<Lens>& base)
{
    return nanobind::class_<ConstantReference>(module_, "ConstantReference", base)
        .def("__repr__", &ConstantReferenceBinding::repr)
        .def("__getitem__", [](const ConstantReference* self, const std::string& name) { return self->operator[](name); })
        .def("__getitem__", [](const ConstantReference* self, size_t index) { return self->operator[](index); })
        .def("type", [](const ConstantReference* self) { return TypeAdapter<Type>(self->type); })
        .def("value", [](const ConstantReference* self)
        {
            const std::type_info& type_info = self->type->info();
            if (type_info == typeid(bool)) return cast<bool>(self->get());
            if (type_info == typeid(uint32_t)) return cast<uint32_t>(self->get());
            if (type_info == typeid(int32_t)) return cast<int32_t>(self->get());
            if (type_info == typeid(float)) return cast<float>(self->get());
            if (type_info == typeid(Vector2)) return cast<Vector2>(self->get());
            if (type_info == typeid(Vector3)) return cast<Vector3>(self->get());
            if (type_info == typeid(std::string)) return cast<std::string>(self->get());
            if (type_info == typeid(uint64_t)) return cast<uint64_t>(self->get());
            if (type_info == typeid(int64_t)) return cast<int64_t>(self->get());
            throw TypeError("no cast for type " + self->type->represent());
        })
        ;
}
