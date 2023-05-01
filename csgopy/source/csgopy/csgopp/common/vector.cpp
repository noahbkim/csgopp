#include "csgopy/csgopp/common/vector.h"

nanobind::class_<Vector2> Vector2Binding::bind(nanobind::module_& module_)
{
    return nanobind::class_<Vector2>(module_, "Vector2")
        .def_prop_ro("x", [](const Vector2* self) { return self->x; })
        .def_prop_ro("y", [](const Vector2* self) { return self->y; });
}

nanobind::class_<Vector3> Vector3Binding::bind(nanobind::module_& module_)
{
    return nanobind::class_<Vector3>(module_, "Vector3")
        .def_prop_ro("x", [](const Vector3* self) { return self->x; })
        .def_prop_ro("y", [](const Vector3* self) { return self->y; })
        .def_prop_ro("z", [](const Vector3* self) { return self->z; });
}
