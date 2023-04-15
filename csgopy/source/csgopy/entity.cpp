#include "entity.h"

#include <nanobind/nanobind.h>

static nanobind::class_<EntityTypeAdapter> bind(nanobind::module_& module_)
{
    return nanobind::class_<EntityTypeAdapter>(module_, "EntityType")
        .def("property", &EntityTypeAdapter::property)
        .def("__getitem__", &EntityTypeAdapter::at_name)
        .def("__getitem__", &EntityTypeAdapter::at_index)
    ;
}
