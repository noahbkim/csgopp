#include <typeindex>

#include <nanobind/nanobind.h>

#include "client.h"
#include "data_table.h"
#include "demo.h"
#include "entity.h"
#include "game_event.h"
#include "object.h"
#include "server_class.h"
#include "string_table.h"
#include "user.h"

using namespace nanobind::literals;

using object::Type;

NB_MODULE(csgopy, module_)
{
    nanobind::exception<csgopp::client::GameError>(module_, "GameError");

    auto object_error = nanobind::exception<object::Error>(module_, "ObjectError");
    nanobind::exception<object::MemberError>(module_, "ObjectMemberError", object_error);
    nanobind::exception<object::IndexError>(module_, "ObjectIndexError", object_error);
    nanobind::exception<object::TypeError>(module_, "ObjectTypeError", object_error);

    DataTableAdapter::bind(module_);
    ServerClassAdapter::bind(module_);
    StringTableAdapter::bind(module_);
    GameEventTypeAdapter::bind(module_);
    GameEventAdapter::bind(module_);
    UserAdapter::bind(module_);

    auto lens_class = LensAdapter::bind(module_);
    EntityLensAdapter::bind(module_, lens_class);
    TypeAdapter<Type>::bind(module_, "Type");
    auto const_reference_class = ConstantReferenceAdapter::bind(module_, lens_class);
    EntityConstantReferenceAdapter::bind(module_, const_reference_class);
    EntityTypeAdapter::bind(module_);
    EntityAdapter::bind(module_);

    nanobind::class_<Vector2>(module_, "Vector2")
    .def_prop_ro("x", [](const Vector2* self) { return self->x; })
        .def_prop_ro("y", [](const Vector2* self) { return self->y; });

    nanobind::class_<Vector3>(module_, "Vector3")
    .def_prop_ro("x", [](const Vector3* self) { return self->x; })
        .def_prop_ro("y", [](const Vector3* self) { return self->y; })
        .def_prop_ro("z", [](const Vector3* self) { return self->z; });

    header::bind(module_);
    ClientAdapter::bind(module_);
}
