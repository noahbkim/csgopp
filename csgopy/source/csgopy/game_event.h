#pragma once

#include <nanobind/nanobind.h>
#include <csgopp/client/game_event.h>
#include "adapter.h"

using csgopp::client::game_event::GameEventType;
using csgopp::client::game_event::GameEvent;

struct GameEventTypeAdapter : public Adapter<const GameEventType>
{
    using Adapter::Adapter;

    static nanobind::class_ <GameEventTypeAdapter> bind(nanobind::module_& module_)
    {
        return nanobind::class_<GameEventTypeAdapter>(module_, "GameEventType");
    }
};

struct GameEventAdapter
{
    static nanobind::class_ <GameEvent> bind(nanobind::module_& module_)
    {
        return nanobind::class_<GameEvent>(module_, "GameEvent");
    }
};
