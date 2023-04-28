#include <typeindex>

#include <nanobind/nanobind.h>

#include "client.h"
#include "common.h"
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
    GameEventBinding::bind(module_);
    UserAdapter::bind(module_);

    auto lens_class = LensBinding::bind(module_);
    EntityLensBinding::bind(module_, lens_class);
    TypeAdapter<Type>::bind(module_, "Type");
    auto const_reference_class = ConstantReferenceBinding::bind(module_, lens_class);
    EntityConstantReferenceBinding::bind(module_, const_reference_class);
    EntityTypeAdapter::bind(module_);
    EntityAdapter::bind(module_);

    HeaderBinding::bind(module_);
    Vector2Binding::bind(module_);
    Vector3Binding::bind(module_);

    ClientAdapter::bind(module_);
}
