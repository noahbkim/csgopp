#pragma once

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <csgopp/client/game_event.h>
#include "adapter.h"
#include "object.h"

using csgopp::client::game_event::GameEventType;
using csgopp::client::game_event::GameEvent;

struct GameEventTypeAdapter : public Adapter<const GameEventType>
{
    using Adapter::Adapter;

    [[nodiscard]] std::string repr() const
    {
        return "GameEvent(id=" + std::to_string(self->id) + ", name=\"" + self->name + "\")";
    }

    static nanobind::class_ <GameEventTypeAdapter> bind(nanobind::module_& module_);
};

struct GameEventBinding
{
    [[nodiscard]] static std::string repr(GameEvent* self)
    {
        return "GameEvent(id=" + std::to_string(self->id) + ", name=\"" + self->type->name + "\")";
    }

    [[nodiscard]] static GameEventType::Id id(GameEvent* self) { return self->id; }
    [[nodiscard]] static GameEventTypeAdapter type(GameEvent* self) { return GameEventTypeAdapter(self->type); }
    [[nodiscard]] static const std::vector<std::string> keys(GameEvent* self) { return self->type->keys(); }

    static nanobind::class_ <GameEvent> bind(nanobind::module_& module_);
};
