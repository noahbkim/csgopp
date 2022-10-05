#pragma once

#include "demo.h"
#include "game/team.h"
#include "netmessages.pb.h"

#define LOCAL(EVENT) _event_##EVENT
#define BEFORE(OBSERVER, EVENT) typename OBSERVER::EVENT LOCAL(EVENT)(*this);
#define AFTER(EVENT, ...) LOCAL(EVENT).handle(*this, __VA_ARGS__);

#define NOOP(NAME, SIMULATION, ...) struct NAME \
{ \
    explicit NAME(SIMULATION&) {} \
    virtual void handle(SIMULATION&, __VA_ARGS__) {} \
}

#define DEBUG(FMT, ...) do \
{ \
    printf("[%s:%d] " FMT "\n", __FILE__, __LINE__, __VA_ARGS__); \
} while (false)

#define ASSERT(CONDITION, FMT, ...) do \
{ \
    if (!(CONDITION)) \
    { \
        fprintf(stderr, "[%s:%d] " FMT "\n", __FILE__, __LINE__, __VA_ARGS__); \
        throw csgopp::game::GameError("failed assertion " #CONDITION); \
    } \
} while (false)

namespace csgopp::game
{

using csgopp::common::reader::Reader;
using csgopp::common::reader::LittleEndian;
using csgopp::demo::VariableSize;

class GameError : public csgopp::error::Error
{
    using Error::Error;
};

template<typename Observer>
class Simulation
{
public:
    struct State
    {
//        ingameTick           int
//        tState               common.TeamState
//        ctState              common.TeamState
//        playersByUserID      map[int]*common.Player            // Maps user-IDs to players
//        playersByEntityID    map[int]*common.Player            // Maps entity-IDs to players
//        playersBySteamID32   map[uint32]*common.Player         // Maps 32-bit-steam-IDs to players
//        playerResourceEntity st.Entity                         // CCSPlayerResource entity instance, contains scoreboard info and more
//        grenadeProjectiles   map[int]*common.GrenadeProjectile // Maps entity-IDs to active nade-projectiles. That's grenades that have been thrown, but have not yet detonated.
//        infernos             map[int]*common.Inferno           // Maps entity-IDs to active infernos.
//        weapons              map[int]*common.Equipment         // Maps entity IDs to weapons. Used to remember what a weapon is (p250 / cz etc.)
//        hostages             map[int]*common.Hostage           // Maps entity-IDs to hostages.
//        entities             map[int]st.Entity                 // Maps entity IDs to entities
//        bomb                 common.Bomb
//        totalRoundsPlayed    int
//        gamePhase            common.GamePhase
//        isWarmupPeriod       bool
//        isMatchStarted       bool
//        lastFlash            lastFlash                              // Information about the last flash that exploded, used to find the attacker and projectile for player_blind events
//        currentDefuser       *common.Player                         // Player currently defusing the bomb, if any
//        currentPlanter       *common.Player                         // Player currently planting the bomb, if any
//        thrownGrenades       map[*common.Player][]*common.Equipment // Information about every player's thrown grenades (from the moment they are thrown to the moment their effect is ended)
//        rules                gameRules
//        demoInfo             demoInfoProvider
    };

    // bombsiteA            bombsite
    // bombsiteB            bombsite
    // equipmentMapping     map[*st.ServerClass]common.EquipmentType        // Maps server classes to equipment-types
    // rawPlayers           map[int]*common.PlayerInfo                      // Maps entity IDs to 'raw' player info
    // modelPreCache        []string                                        // Used to find out whether a weapon is a p250 or cz for example (same id)
    // triggers             map[int]*boundingBoxInformation                 // Maps entity IDs to triggers (used for bombsites)
    // gameEventDescs       map[int32]*msg.CSVCMsg_GameEventListDescriptorT // Maps game-event IDs to descriptors
    // grenadeModelIndices  map[int]common.EquipmentType                    // Used to map model indices to grenades (used for grenade projectiles)
    // stringTables         []*msg.CSVCMsg_CreateStringTable                // Contains all created sendtables, needed when updating them
    // delayedEventHandlers []func()

    explicit Simulation(Reader& reader);
    explicit Simulation(demo::Header&& header);

    bool advance(Reader& reader);
        void advance_packets(Reader& reader);
            int32_t advance_packet(Reader& reader);
        void advance_console_command(Reader& reader);
        void advance_user_command(Reader& reader);
        void advance_data_tables(Reader& reader);
        void advance_string_tables(Reader& reader);

    GET(header, const&);
    GET(state, const&);
    GET(cursor);
    GET(tick);

    Observer observer;

protected:


private:
    demo::Header _header;
    State _state;
    size_t _cursor = 0;
    size_t _tick = 0;
};

template<typename Observer>
struct ObserverBase
{
    using Simulation = Simulation<Observer>;

    NOOP(Frame, Simulation, demo::Command);
    NOOP(Packet, Simulation, int32_t);
};

template<typename Observer>
Simulation<Observer>::Simulation(demo::Header&& header) : _header(header) {}

template<typename Observer>
Simulation<Observer>::Simulation(Reader& reader) : Simulation(demo::Header::deserialize(reader)) {}

template<typename Observer>
bool Simulation<Observer>::advance(Reader& reader)
{
    BEFORE(Observer, Frame);
    char command = reader.read<char>();
    this->_tick = reader.read<int32_t, LittleEndian>();
    reader.skip(1);  // player slot

    switch (command)
    {
        // Handle packets
        case static_cast<char>(demo::Command::SIGN_ON):
            this->advance_packets(reader);
            AFTER(Frame, demo::Command::SIGN_ON);
            return true;
        case static_cast<char>(demo::Command::PACKET):
            this->advance_packets(reader);
            AFTER(Frame, demo::Command::PACKET);
            return true;

        // Exists only to synchronize demo with current tick in absence of activity
        case static_cast<char>(demo::Command::SYNC_TICK):
            AFTER(Frame, demo::Command::SYNC_TICK);
            return true;

        // We ignore these
        case static_cast<char>(demo::Command::CONSOLE_COMMAND):
            this->advance_console_command(reader);
            AFTER(Frame, demo::Command::CONSOLE_COMMAND);
            return true;
        case static_cast<char>(demo::Command::USER_COMMAND):
            this->advance_user_command(reader);
            AFTER(Frame, demo::Command::CONSOLE_COMMAND);
            return true;

        // Update sendtables
        case static_cast<char>(demo::Command::DATA_TABLES):
            this->advance_data_tables(reader);
            AFTER(Frame, demo::Command::DATA_TABLES);
            return true;
        case static_cast<char>(demo::Command::STRING_TABLES):
            this->advance_string_tables(reader);
            AFTER(Frame, demo::Command::STRING_TABLES);
            return true;

        // Indicate the end of the demo
        case static_cast<char>(demo::Command::STOP):
            AFTER(Frame, demo::Command::STOP);
            return false;

        // Failure conditions
        case static_cast<char>(demo::Command::CUSTOM_DATA):
            throw GameError("encountered unexpected CUSTOM_DATA event!");
        default:
            throw GameError("encountered unknown command " + std::to_string(command));
    }
}

template<typename Observer>
void Simulation<Observer>::advance_packets(Reader& reader)
{
    reader.skip(152 + 4 + 4);
    int32_t size = reader.read<int32_t, LittleEndian>();
    int32_t cursor = 0;

    while (cursor < size)
    {
        cursor += this->advance_packet(reader);
    }

    ASSERT(cursor == size, "failed to read to expected packet alignment");
}

template<typename Observer>
int32_t Simulation<Observer>::advance_packet(Reader& reader)
{
    BEFORE(Observer, Packet);
    VariableSize<int32_t, int32_t> command = VariableSize<int32_t, int32_t>::deserialize(reader);
    VariableSize<int32_t, int32_t> size = VariableSize<int32_t, int32_t>::deserialize(reader);

    using namespace csgo::message::net;
    switch (command.value)
    {
        case NET_Messages::net_Tick:
            break;
        case NET_Messages::net_StringCmd:
            break;
        case NET_Messages::net_SetConVar:
            break;
        case NET_Messages::net_SignonState:
            break;
        case SVC_Messages::svc_ServerInfo:
            break;
        case SVC_Messages::svc_ClassInfo:
            break;
        case SVC_Messages::svc_CreateStringTable:
            break;
        case SVC_Messages::svc_UpdateStringTable:
            break;
        case SVC_Messages::svc_VoiceInit:
            break;
        case SVC_Messages::svc_Sounds:
            break;
        case SVC_Messages::svc_SetView:
            break;
        case SVC_Messages::svc_BSPDecal:
            break;
        case SVC_Messages::svc_UserMessage:
            break;
        case SVC_Messages::svc_GameEvent:
            break;
        case SVC_Messages::svc_PacketEntities:
            break;
        case SVC_Messages::svc_TempEntities:
            break;
        case SVC_Messages::svc_Prefetch:
            break;
        case SVC_Messages::svc_GameEventList:
            break;
        default:
            DEBUG("unrecognized message %s", demo::describe_net_message(command.value));
    }

    reader.skip(size.value);

    AFTER(Packet, command.value);
    return command.size + size.size + size.value;
}

template<typename Observer>
void Simulation<Observer>::advance_console_command(Reader& reader)
{
    reader.skip(reader.read<int32_t, LittleEndian>());
}

template<typename Observer>
void Simulation<Observer>::advance_user_command(Reader& reader)
{
    reader.skip(4);
    reader.skip(reader.read<int32_t, LittleEndian>());
}

template<typename Observer>
void Simulation<Observer>::advance_data_tables(Reader& reader)
{
    reader.skip(reader.read<int32_t, LittleEndian>());
}

template<typename Observer>
void Simulation<Observer>::advance_string_tables(Reader& reader)
{
    reader.skip(reader.read<int32_t, LittleEndian>());
}

}
