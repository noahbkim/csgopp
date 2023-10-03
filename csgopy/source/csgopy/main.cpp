#include <typeindex>

#include <nanobind/nanobind.h>

#include "csgopy/csgopp/client.h"
#include "csgopy/csgopp/client/data_table.h"
#include "csgopy/csgopp/client/entity.h"
#include "csgopy/csgopp/client/game_event.h"
#include "csgopy/csgopp/client/server_class.h"
#include "csgopy/csgopp/client/string_table.h"
#include "csgopy/csgopp/client/user.h"
#include "csgopy/csgopp/common/vector.h"
#include "csgopy/csgopp/demo.h"
#include "csgopy/objective/object.h"

using namespace nanobind::literals;

using objective::Type;

NB_MODULE(csgopy, module_)
{
    nanobind::exception<csgopp::client::GameError>(module_, "GameError");

    auto object_error = nanobind::exception<objective::Error>(module_, "ObjectError");
    nanobind::exception<objective::MemberError>(module_, "ObjectMemberError", object_error);
    nanobind::exception<objective::IndexError>(module_, "ObjectIndexError", object_error);
    nanobind::exception<objective::TypeError>(module_, "ObjectTypeError", object_error);

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
