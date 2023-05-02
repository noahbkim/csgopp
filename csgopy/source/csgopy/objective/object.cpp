#include "csgopy/objective/object.h"

ConstantReferenceBinding::CasterMap ConstantReferenceBinding::casters{
    {&typeid(bool),        &cast<bool>},
    {&typeid(uint32_t),    &cast<uint32_t>},
    {&typeid(int32_t),     &cast<int32_t>},
    {&typeid(float),       &cast<float>},
    {&typeid(Vector2),     &cast<Vector2>},
    {&typeid(Vector3),     &cast<Vector3>},
    {&typeid(std::string), &cast<std::string>},
    {&typeid(uint64_t),    &cast<uint64_t>},
    {&typeid(int64_t),     &cast<int64_t>},
};

nanobind::class_<ConstantReference> ConstantReferenceBinding::bind(nanobind::module_& module_, nanobind::class_<Lens>& base)
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
                Caster caster = ConstantReferenceBinding::casters[&value_type->info()];
                return caster(self->data.get());
            }
            else
            {
                throw TypeError("cast is only available for values!");
            }
        })
        ;
}
