#include "server_class.h"
#include "entity.h"

nanobind::class_<ServerClassAdapter> ServerClassAdapter::bind(nanobind::module_& module_)
{
    return nanobind::class_<ServerClassAdapter>(module_, "ServerClass")
        .def("__repr__", &ServerClassAdapter::repr)
        .def("index", &ServerClassAdapter::index)
        .def("name", &ServerClassAdapter::name)
        .def("type", [](const ServerClassAdapter* adapter) { return EntityTypeAdapter(adapter->self->type()); })
    ;
}
