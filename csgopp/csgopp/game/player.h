#pragma once

#include <cstdint>
#include <string>

#include "../common/vector.h"
#include "../common/id.h"
#include "../common/interface.h"
#include "entity.h"

namespace csgopp::game
{

using csgopp::common::id::IdBase;
using csgopp::common::vector::Vector3;

class Team;

class Player
{
public:
    using Id = IdBase<Player>;

    class State
    {
    friend Player;
    private:
        Vector3 _last_alive_position{};
        // inventory
        std::string _name{};
        uint16_t _reserve_ammo_count[32]{};  // For each weapon
        int32_t _entity_id{};
        Entity _entity{};
        size_t _flash_tick{};
        float _flash_duration{};
        Team* _team{};
        bool _is_bot{};
        bool _is_connected{};
        bool _is_defusing{};
        bool _is_planting{};
        bool _is_reloading{};
    };

protected:
    GET(id);
    GET(steam_id);
    GET(user_id);
    GET(state, const&);

private:
    Id _id;
    uint64_t _steam_id{};
    int32_t _user_id{};
    State _state{};
};

};
