#include "user.h"

nanobind::class_<UserAdapter> UserAdapter::bind(nanobind::module_& module_)
{
    return nanobind::class_<UserAdapter>(module_, "User")
        .def("__repr__", &UserAdapter::repr)
        .def_prop_ro("index", &UserAdapter::index)
        .def_prop_ro("version", &UserAdapter::version)
        .def_prop_ro("xuid", &UserAdapter::xuid)
        .def_prop_ro("name", &UserAdapter::name)
        .def_prop_ro("id", &UserAdapter::id)
        .def_prop_ro("guid", &UserAdapter::guid)
        .def_prop_ro("friends_id", &UserAdapter::friends_id)
        .def_prop_ro("friends_name", &UserAdapter::friends_name)
        .def_prop_ro("is_fake", &UserAdapter::is_fake)
        .def_prop_ro("is_hltv", &UserAdapter::is_hltv)
        .def_prop_ro("custom_files", &UserAdapter::custom_files)
        .def_prop_ro("files_downloaded", &UserAdapter::files_downloaded)
        ;
}
