#pragma once

#include <vector>
#include "../common/id.h"
#include "player.h"

namespace csgopp::model
{

class Team
{
public:
    using Id = IdBase<Team>;

    enum class Type
    {
        TeamUnassigned = 0,
        TeamSpectators = 1,
        TeamTerrorists = 2,
        TeamCounterTerrorists = 3,
    } type;

    class State
    {

    };

private:
    Id _id;
    Type _type;
    std::vector<Player*> _players;

    State _state;
};

}
