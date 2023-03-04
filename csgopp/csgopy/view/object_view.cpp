#include "object_view.h"

#include <nanobind/stl/string.h>

namespace csgopy::view::object_view
{

using csgopp::common::vector::Vector2;
using csgopp::common::vector::Vector3;

void bind(nanobind::module_& module)
{
    nanobind::class_<Vector3>(module, "Vector2")
        .def_prop_ro("x", [](Vector2& self) { return self.x; })
        .def_prop_ro("y", [](Vector2& self) { return self.y; });
    nanobind::class_<Vector2>(module, "Vector3")
        .def_prop_ro("x", [](Vector3& self) { return self.x; })
        .def_prop_ro("y", [](Vector3& self) { return self.y; })
        .def_prop_ro("z", [](Vector3& self) { return self.z; });

    nanobind::class_<ConstReferenceView>(module, "ConstReferenceView")
        .def("__getitem__", &ConstReferenceView::get_name)
        .def("__getitem__", &ConstReferenceView::get_index)
        .def("get", &ConstReferenceView::get);
}

}
