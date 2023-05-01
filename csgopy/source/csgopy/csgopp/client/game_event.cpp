#include "csgopy/csgopp/client/game_event.h"

nanobind::class_ <GameEventTypeAdapter> GameEventTypeAdapter::bind(nanobind::module_& module_)
{
    return nanobind::class_<GameEventTypeAdapter>(module_, "GameEventType")
        .def("__repr__", &GameEventTypeAdapter::repr)
        ;
}

nanobind::class_<GameEvent> GameEventBinding::bind(nanobind::module_& module_)
{
    auto base = InstanceAdapter<GameEventType>::bind(module_, "GameEventTypeInstance");
    return nanobind::class_<GameEvent>(module_, "GameEvent", base)
        .def("__repr__", &GameEventBinding::repr)
        .def_prop_ro("id", &GameEventBinding::id)
        .def_prop_ro("type", &GameEventBinding::type)
        .def("keys", &GameEventBinding::keys)
        ;
}
