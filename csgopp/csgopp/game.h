#pragma once

#include <string>

#include "demo.h"
#include "game/team.h"
#include "game/send_table.h"
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
using csgopp::error::GameError;

template<typename Observer>
class Simulation
{
public:
    struct State
    {

    };

    struct Data
    {
        std::vector<SendTable> send_tables;
    };

    explicit Simulation(Reader& reader);
    explicit Simulation(demo::Header&& header);

    virtual bool advance(Reader& reader);
        virtual void advance_packets(Reader& reader);
            virtual int32_t advance_packet(Reader& reader);
                virtual int32_t advance_packet_nop(Reader& reader);
                virtual int32_t advance_packet_disconnect(Reader& reader);
                virtual int32_t advance_packet_file(Reader& reader);
                virtual int32_t advance_packet_split_screen_user(Reader& reader);
                virtual int32_t advance_packet_tick(Reader& reader);
                virtual int32_t advance_packet_string_command(Reader& reader);
                virtual int32_t advance_packet_set_console_variable(Reader& reader);
                virtual int32_t advance_packet_sign_on_state(Reader& reader);
                virtual int32_t advance_packet_server_info(Reader& reader);
                virtual int32_t advance_packet_send_table(Reader& reader);
                virtual int32_t advance_packet_class_info(Reader& reader);
                virtual int32_t advance_packet_set_pause(Reader& reader);
                virtual int32_t advance_packet_create_string_table(Reader& reader);
                virtual int32_t advance_packet_update_string_table(Reader& reader);
                virtual int32_t advance_packet_voice_initialization(Reader& reader);
                virtual int32_t advance_packet_voice_data(Reader& reader);
                virtual int32_t advance_packet_print(Reader& reader);
                virtual int32_t advance_packet_sounds(Reader& reader);
                virtual int32_t advance_packet_set_view(Reader& reader);
                virtual int32_t advance_packet_fix_angle(Reader& reader);
                virtual int32_t advance_packet_crosshair_angle(Reader& reader);
                virtual int32_t advance_packet_bsp_decal(Reader& reader);
                virtual int32_t advance_packet_split_screen(Reader& reader);
                virtual int32_t advance_packet_user_message(Reader& reader);
                virtual int32_t advance_packet_entity_message(Reader& reader);
                virtual int32_t advance_packet_game_event(Reader& reader);
                virtual int32_t advance_packet_packet_entities(Reader& reader);
                virtual int32_t advance_packet_temporary_entities(Reader& reader);
                virtual int32_t advance_packet_prefetch(Reader& reader);
                virtual int32_t advance_packet_menu(Reader& reader);
                virtual int32_t advance_packet_game_event_list(Reader& reader);
                virtual int32_t advance_packet_get_console_variable_value(Reader& reader);
                virtual int32_t advance_packet_paintmap_data(Reader& reader);
                virtual int32_t advance_packet_command_key_values(Reader& reader);
                virtual int32_t advance_packet_encrypted_data(Reader& reader);
                virtual int32_t advance_packet_hltv_replay(Reader& reader);
                virtual int32_t advance_packet_broadcast_command(Reader& reader);
                virtual int32_t advance_packet_player_avatar_data(Reader& reader);
                virtual int32_t advance_packet_unknown(Reader& reader, int32_t command);
        virtual void advance_console_command(Reader& reader);
        virtual void advance_user_command(Reader& reader);
        virtual void advance_data_tables(Reader& reader);
        virtual void advance_string_tables(Reader& reader);
        virtual void advance_custom_data(Reader& reader);
        virtual bool advance_unknown(Reader& reader, char command);

    GET(header, const&);
    GET(state, const&);
    GET(data, const&);
    GET(cursor);
    GET(tick);

    Observer observer;

protected:
    demo::Header _header;
    State _state;
    Data _data;
    size_t _cursor = 0;
    size_t _tick = 0;
};

template<typename Observer>
struct ObserverBase
{
    using Simulation = Simulation<Observer>;

    NOOP(Frame, Simulation, demo::Command);
    NOOP(Packet, Simulation, int32_t);
    NOOP(PacketSetConVar, Simulation, const csgo::message::net::CNETMsg_SetConVar&);
    NOOP(PacketServerInfo, Simulation, const csgo::message::net::CSVCMsg_ServerInfo&);
    NOOP(PacketCreateStringTable, Simulation, const csgo::message::net::CSVCMsg_CreateStringTable&);
    NOOP(PacketUpdateStringTable, Simulation, const csgo::message::net::CSVCMsg_UpdateStringTable&);
    NOOP(PacketSounds, Simulation, const csgo::message::net::CSVCMsg_Sounds&);
    NOOP(PacketUserMessage, Simulation, const csgo::message::net::CSVCMsg_UserMessage&);
    NOOP(PacketGameEvent, Simulation, const csgo::message::net::CSVCMsg_GameEvent&);
    NOOP(PacketPacketEntities, Simulation, const csgo::message::net::CSVCMsg_PacketEntities&);
    NOOP(PacketGameEventList, Simulation, const csgo::message::net::CSVCMsg_GameEventList&);
};

#define SIMULATION(TYPE, NAME, ...) template<typename Observer> TYPE Simulation<Observer>::NAME(__VA_ARGS__)

template<typename Observer>
Simulation<Observer>::Simulation(demo::Header&& header) : _header(header) {}

template<typename Observer>
Simulation<Observer>::Simulation(Reader& reader) : Simulation(demo::Header::deserialize(reader)) {}

SIMULATION(bool, advance, Reader& reader)
{
    BEFORE(Observer, Frame);
    char command = reader.read<char>();
    this->_tick = reader.read<int32_t, LittleEndian>();
    reader.skip(1);  // player slot

    switch (command)
    {
        case static_cast<char>(demo::Command::SIGN_ON):
            this->advance_packets(reader);
            AFTER(Frame, demo::Command::SIGN_ON);
            return true;
        case static_cast<char>(demo::Command::PACKET):
            this->advance_packets(reader);
            AFTER(Frame, demo::Command::PACKET);
            return true;
        case static_cast<char>(demo::Command::SYNC_TICK):
            AFTER(Frame, demo::Command::SYNC_TICK);
            return true;
        case static_cast<char>(demo::Command::CONSOLE_COMMAND):
            this->advance_console_command(reader);
            AFTER(Frame, demo::Command::CONSOLE_COMMAND);
            return true;
        case static_cast<char>(demo::Command::USER_COMMAND):
            this->advance_user_command(reader);
            AFTER(Frame, demo::Command::CONSOLE_COMMAND);
            return true;
        case static_cast<char>(demo::Command::DATA_TABLES):
            this->advance_data_tables(reader);
            AFTER(Frame, demo::Command::DATA_TABLES);
            return true;
        case static_cast<char>(demo::Command::STOP):
            AFTER(Frame, demo::Command::STOP);
            return false;
        case static_cast<char>(demo::Command::CUSTOM_DATA):
            this->advance_custom_data(reader);
            AFTER(Frame, demo::Command::CUSTOM_DATA);
            return true;
        case static_cast<char>(demo::Command::STRING_TABLES):
            this->advance_string_tables(reader);
            AFTER(Frame, demo::Command::STRING_TABLES);
            return true;
        default:
            return this->advance_unknown(reader, command);
    }
}

SIMULATION(void, advance_packets, Reader& reader)
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

#define PACKET(COMMAND, CALLBACK) case COMMAND: size += CALLBACK(reader); break;

SIMULATION(int32_t, advance_packet, Reader& reader)
{
    BEFORE(Observer, Packet);
    auto [command, size] = VariableSize<int32_t, int32_t>::deserialize(reader);

    using namespace csgo::message::net;
    switch (command)
    {
        PACKET(NET_Messages::net_NOP, this->advance_packet_nop);
        PACKET(NET_Messages::net_Disconnect, this->advance_packet_disconnect);
        PACKET(NET_Messages::net_File, this->advance_packet_file);
        PACKET(NET_Messages::net_SplitScreenUser, this->advance_packet_split_screen_user);
        PACKET(NET_Messages::net_Tick, this->advance_packet_tick);
        PACKET(NET_Messages::net_StringCmd, this->advance_packet_string_command);
        PACKET(NET_Messages::net_SetConVar, this->advance_packet_set_console_variable);
        PACKET(NET_Messages::net_SignonState, this->advance_packet_sign_on_state);
        PACKET(SVC_Messages::svc_ServerInfo, this->advance_packet_server_info);
        PACKET(SVC_Messages::svc_SendTable, this->advance_packet_send_table);
        PACKET(SVC_Messages::svc_ClassInfo, this->advance_packet_class_info);
        PACKET(SVC_Messages::svc_SetPause, this->advance_packet_set_pause);
        PACKET(SVC_Messages::svc_CreateStringTable, this->advance_packet_create_string_table);
        PACKET(SVC_Messages::svc_UpdateStringTable, this->advance_packet_update_string_table);
        PACKET(SVC_Messages::svc_VoiceInit, this->advance_packet_voice_initialization);
        PACKET(SVC_Messages::svc_VoiceData, this->advance_packet_voice_data);
        PACKET(SVC_Messages::svc_Print, this->advance_packet_print);
        PACKET(SVC_Messages::svc_Sounds, this->advance_packet_sounds);
        PACKET(SVC_Messages::svc_SetView, this->advance_packet_set_view);
        PACKET(SVC_Messages::svc_FixAngle, this->advance_packet_fix_angle);
        PACKET(SVC_Messages::svc_CrosshairAngle, this->advance_packet_crosshair_angle);
        PACKET(SVC_Messages::svc_BSPDecal, this->advance_packet_bsp_decal);
        PACKET(SVC_Messages::svc_SplitScreen, this->advance_packet_split_screen);
        PACKET(SVC_Messages::svc_UserMessage, this->advance_packet_user_message);
        PACKET(SVC_Messages::svc_EntityMessage, this->advance_packet_entity_message);
        PACKET(SVC_Messages::svc_GameEvent, this->advance_packet_game_event);
        PACKET(SVC_Messages::svc_PacketEntities, this->advance_packet_packet_entities);
        PACKET(SVC_Messages::svc_TempEntities, this->advance_packet_temporary_entities);
        PACKET(SVC_Messages::svc_Prefetch, this->advance_packet_prefetch);
        PACKET(SVC_Messages::svc_Menu, this->advance_packet_menu);
        PACKET(SVC_Messages::svc_GameEventList, this->advance_packet_game_event_list);
        PACKET(SVC_Messages::svc_GetCvarValue, this->advance_packet_get_console_variable_value);
        PACKET(SVC_Messages::svc_PaintmapData, this->advance_packet_paintmap_data);
        PACKET(SVC_Messages::svc_CmdKeyValues, this->advance_packet_command_key_values);
        PACKET(SVC_Messages::svc_EncryptedData, this->advance_packet_encrypted_data);
        PACKET(SVC_Messages::svc_HltvReplay, this->advance_packet_hltv_replay);
        PACKET(SVC_Messages::svc_Broadcast_Command, this->advance_packet_broadcast_command);
        PACKET(NET_Messages::net_PlayerAvatarData, this->advance_packet_player_avatar_data);
        default: size += this->advance_packet_unknown(reader, command);
    }

    AFTER(Packet, command);
    return size;
}

SIMULATION(void, advance_console_command, Reader& reader)
{
    reader.skip(reader.read<int32_t, LittleEndian>());
}

SIMULATION(void, advance_user_command, Reader& reader)
{
    reader.skip(4);
    reader.skip(reader.read<int32_t, LittleEndian>());
}

SIMULATION(void, advance_data_tables, Reader& reader)
{
    int32_t size = reader.read<int32_t, LittleEndian>();
//    size_t start = reader.tell();
    reader.skip(size);

//    VariableSize<int32_t, int32_t> type = VariableSize<int32_t, int32_t>::deserialize(reader);
//    if (type.value != csgo::message::net::SVC_Messages::svc_SendTable)
//    {
//        throw GameError("got unexpected " + std::string(demo::describe_net_message(type.value)));
//    }

//    // Actual read the SendTable
//    this->_data.send_tables.emplace_back(SendTable::deserialize(reader));
//
//    // REMOVE
//    if (reader.tell() < start + size)
//    {
//        reader.skip(reader.tell() - (start + size));
//    }

//    ASSERT(reader.tell() == start + size, "cursor mismatch!");
}

SIMULATION(void, advance_string_tables, Reader& reader)
{
    reader.skip(reader.read<int32_t, LittleEndian>());
}

SIMULATION(void, advance_custom_data, Reader& reader)
{
    throw GameError("encountered unexpected CUSTOM_DATA event!");
}

SIMULATION(bool, advance_unknown, Reader& reader, char command)
{
    throw GameError("encountered unknown command " + std::to_string(command));
}

inline int32_t advance_packet_skip(Reader& reader)
{
    VariableSize<int32_t, int32_t> size = VariableSize<int32_t, int32_t>::deserialize(reader);
    reader.skip(size.value);
    return size.size + size.value;
}

#define PACKET_SKIP() { return advance_packet_skip(reader); }

SIMULATION(int32_t, advance_packet_nop, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_disconnect, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_file, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_split_screen_user, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_tick, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_string_command, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_set_console_variable, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_sign_on_state, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_server_info, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_send_table, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_class_info, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_set_pause, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_create_string_table, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_update_string_table, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_voice_initialization, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_voice_data, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_print, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_sounds, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_set_view, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_fix_angle, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_crosshair_angle, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_bsp_decal, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_split_screen, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_user_message, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_entity_message, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_game_event, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_packet_entities, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_temporary_entities, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_prefetch, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_menu, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_game_event_list, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_get_console_variable_value, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_paintmap_data, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_command_key_values, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_encrypted_data, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_hltv_replay, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_broadcast_command, Reader& reader) PACKET_SKIP()
SIMULATION(int32_t, advance_packet_player_avatar_data, Reader& reader) PACKET_SKIP()

SIMULATION(int32_t, advance_packet_unknown, Reader& reader, int32_t command)
{
    throw GameError("unrecognized message " + std::string(demo::describe_net_message(command)));
}

}
