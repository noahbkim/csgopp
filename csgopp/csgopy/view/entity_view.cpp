#include "entity_view.h"

#include <nanobind/stl/string.h>

namespace csgopy::view::entity_view
{

void bind(nanobind::module_& module)
{
    nanobind::class_<EntityView>(module, "EntityView")
        .def_prop_ro("id", &EntityView::id)
        .def("__getitem__", &EntityView::get_name)
        .def("__getitem__", &EntityView::get_index)
        .def("get", &EntityView::get);
}

}
