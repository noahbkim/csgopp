#pragma once

#include <string>
#include <absl/container/flat_hash_map.h>
#include <google/protobuf/io/coded_stream.h>

#include "demo.h"
#include "game/team.h"
#include "network.h"
#include "network/send_table.h"
#include "network/server_class.h"
#include "netmessages.pb.h"

#define LOCAL(EVENT) _event_##EVENT
#define BEFORE(OBSERVER, EVENT) typename OBSERVER::EVENT LOCAL(EVENT)(*this);
#define AFTER(EVENT, ...) LOCAL(EVENT).handle(*this, __VA_ARGS__);

#define NOOP(NAME, SIMULATION, ...) struct NAME \
{ \
    explicit NAME(SIMULATION&) {} \
    virtual void handle(SIMULATION&, __VA_ARGS__) {} \
}

namespace csgopp::game
{

using google::protobuf::io::CodedInputStream;
using csgopp::error::GameError;
using csgopp::network::Database;
using csgopp::network::SendTable;
using csgopp::network::ServerClass;

template<typename Observer>
class Simulation
{
public:
    struct State
    {

    };

    struct Network
    {
        Database<SendTable> send_tables;
        Database<ServerClass> server_classes;
    };

    explicit Simulation(CodedInputStream& stream);

    virtual bool advance(CodedInputStream& stream);
        virtual void advance_packets(CodedInputStream& stream);
            virtual void advance_packet(CodedInputStream& stream);
                virtual void advance_packet_nop(CodedInputStream& stream);
                virtual void advance_packet_disconnect(CodedInputStream& stream);
                virtual void advance_packet_file(CodedInputStream& stream);
                virtual void advance_packet_split_screen_user(CodedInputStream& stream);
                virtual void advance_packet_tick(CodedInputStream& stream);
                virtual void advance_packet_string_command(CodedInputStream& stream);
                virtual void advance_packet_set_console_variable(CodedInputStream& stream);
                virtual void advance_packet_sign_on_state(CodedInputStream& stream);
                virtual void advance_packet_server_info(CodedInputStream& stream);
                virtual void advance_packet_send_table(CodedInputStream& stream);
                virtual void advance_packet_class_info(CodedInputStream& stream);
                virtual void advance_packet_set_pause(CodedInputStream& stream);
                virtual void advance_packet_create_string_table(CodedInputStream& stream);
                virtual void advance_packet_update_string_table(CodedInputStream& stream);
                virtual void advance_packet_voice_initialization(CodedInputStream& stream);
                virtual void advance_packet_voice_data(CodedInputStream& stream);
                virtual void advance_packet_print(CodedInputStream& stream);
                virtual void advance_packet_sounds(CodedInputStream& stream);
                virtual void advance_packet_set_view(CodedInputStream& stream);
                virtual void advance_packet_fix_angle(CodedInputStream& stream);
                virtual void advance_packet_crosshair_angle(CodedInputStream& stream);
                virtual void advance_packet_bsp_decal(CodedInputStream& stream);
                virtual void advance_packet_split_screen(CodedInputStream& stream);
                virtual void advance_packet_user_message(CodedInputStream& stream);
                virtual void advance_packet_entity_message(CodedInputStream& stream);
                virtual void advance_packet_game_event(CodedInputStream& stream);
                virtual void advance_packet_packet_entities(CodedInputStream& stream);
                virtual void advance_packet_temporary_entities(CodedInputStream& stream);
                virtual void advance_packet_prefetch(CodedInputStream& stream);
                virtual void advance_packet_menu(CodedInputStream& stream);
                virtual void advance_packet_game_event_list(CodedInputStream& stream);
                virtual void advance_packet_get_console_variable_value(CodedInputStream& stream);
                virtual void advance_packet_paintmap_data(CodedInputStream& stream);
                virtual void advance_packet_command_key_values(CodedInputStream& stream);
                virtual void advance_packet_encrypted_data(CodedInputStream& stream);
                virtual void advance_packet_hltv_replay(CodedInputStream& stream);
                virtual void advance_packet_broadcast_command(CodedInputStream& stream);
                virtual void advance_packet_player_avatar_data(CodedInputStream& stream);
                virtual void advance_packet_unknown(CodedInputStream& stream, int32_t command);
        virtual void advance_console_command(CodedInputStream& stream);
        virtual void advance_user_command(CodedInputStream& stream);
        virtual void advance_data_tables(CodedInputStream& stream);
        virtual void advance_string_tables(CodedInputStream& stream);
        virtual void advance_custom_data(CodedInputStream& stream);
        virtual bool advance_unknown(CodedInputStream& stream, char command);

    GET(header, const&);
    GET(state, const&);
    GET(data, const&);
    GET(cursor);
    GET(tick);

    Observer observer;

protected:
    demo::Header _header;
    State _state;
    Network _network;
    uint32_t _cursor{0};
    uint32_t _tick{0};
};

template<typename Observer>
struct ObserverBase
{
    using Simulation = Simulation<Observer>;

    NOOP(Frame, Simulation, uint8_t);
    NOOP(Packet, Simulation, int32_t);
    NOOP(SendTableCreate, Simulation, const SendTable&);
    NOOP(ServerClassCreate, Simulation, const ServerClass&);
};

#define SIMULATION(TYPE, NAME, ...) template<typename Observer> TYPE Simulation<Observer>::NAME(__VA_ARGS__)

template<typename Observer>
Simulation<Observer>::Simulation(CodedInputStream& stream) : _header(stream) {}

SIMULATION(bool, advance, CodedInputStream& stream)
{
    bool ok = true;
    BEFORE(Observer, Frame);

    char command;
    OK(stream.ReadRaw(&command, 1));
    OK(stream.ReadLittleEndian32(&this->_tick));
    OK(stream.Skip(1));  // player slot

    switch (command)
    {
        case demo::Command::SIGN_ON:
            this->advance_packets(stream);
            break;
        case demo::Command::PACKET:
            this->advance_packets(stream);
            break;
        case demo::Command::SYNC_TICK:
            break;
        case demo::Command::CONSOLE_COMMAND:
            this->advance_console_command(stream);
            break;
        case demo::Command::USER_COMMAND:
            this->advance_user_command(stream);
            break;
        case demo::Command::DATA_TABLES:
            this->advance_data_tables(stream);
            break;
        case demo::Command::STOP:
            ok = false;
            break;
        case demo::Command::CUSTOM_DATA:
            this->advance_custom_data(stream);
            break;
        case demo::Command::STRING_TABLES:
            this->advance_string_tables(stream);
            break;
        default:
            this->advance_unknown(stream, command);
    }

    this->_cursor += 1;
    AFTER(Frame, command);
    return ok;
}

SIMULATION(void, advance_packets, CodedInputStream& stream)
{
    // Arbitrary player data, seems useless
    OK(stream.Skip(152 + 4 + 4));

    uint32_t size;
    OK(stream.ReadLittleEndian32(&size));
    CodedInputStream::Limit limit = stream.PushLimit(size);

    while (stream.BytesUntilLimit() > 0)
    {
        this->advance_packet(stream);
    }

    OK(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

#define PACKET(COMMAND, CALLBACK) case COMMAND: CALLBACK(stream); break;

SIMULATION(void, advance_packet, CodedInputStream& stream)
{
    BEFORE(Observer, Packet);
    uint32_t command = stream.ReadTag();

    switch (command)
    {
        using namespace csgo::message::net;
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
        default: this->advance_packet_unknown(stream, command);
    }

    AFTER(Packet, command);
}

SIMULATION(void, advance_console_command, CodedInputStream& stream)
{
    uint32_t size;
    OK(stream.ReadLittleEndian32(&size));
    OK(stream.Skip(size));
}

SIMULATION(void, advance_user_command, CodedInputStream& stream)
{
    OK(stream.Skip(4));
    uint32_t size;
    OK(stream.ReadLittleEndian32(&size));
    OK(stream.Skip(size));
}

SIMULATION(void, advance_data_tables, CodedInputStream& stream)
{
    uint32_t size;
    OK(stream.ReadLittleEndian32(&size));
    CodedInputStream::Limit limit = stream.PushLimit(size);

    this->advance_packet_send_table(stream);

    OK(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

SIMULATION(void, advance_string_tables, CodedInputStream& stream)
{
    uint32_t size;
    OK(stream.ReadLittleEndian32(&size));
    OK(stream.Skip(size));
}

SIMULATION(void, advance_custom_data, CodedInputStream& stream)
{
    throw GameError("encountered unexpected CUSTOM_DATA event!");
}

SIMULATION(bool, advance_unknown, CodedInputStream& stream, char command)
{
    throw GameError("encountered unknown command " + std::to_string(command));
}

inline void advance_packet_skip(CodedInputStream& stream)
{
    uint32_t size;
    OK(stream.ReadVarint32(&size));
    OK(stream.Skip(static_cast<int32_t>(size)));
}

#define PACKET_SKIP() { advance_packet_skip(stream); }

SIMULATION(void, advance_packet_nop, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_disconnect, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_file, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_split_screen_user, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_tick, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_string_command, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_set_console_variable, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_sign_on_state, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_server_info, CodedInputStream& stream) PACKET_SKIP()

SIMULATION(void, advance_packet_send_table, CodedInputStream& stream)
{
    csgo::message::net::CSVCMsg_SendTable data;
    do
    {
        // We're gonna manually pull the protobuf here in order to determine whether it's the terminator. This deviates
        // from my general rule of opening the event before absolutely any handling, but it's better than firing on the
        // empty terminator event.
        OK(stream.ExpectTag(csgo::message::net::SVC_Messages::svc_SendTable));
        CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
        OK(limit > 0);

        // Break if we're at the end of the send_table.
        data.ParseFromCodedStream(&stream);

        // Actually do event handling if we're not at the terminator.
        if (!data.is_end())
        {
            BEFORE(Observer, SendTableCreate);
            std::unique_ptr<SendTable> send_table = std::make_unique<SendTable>();
            send_table->deserialize(data);
            const SendTable& reference = *send_table;
            this->_network.send_tables.emplace(send_table->name, std::move(send_table));
            AFTER(SendTableCreate, reference);
        }

        // Related to inline parsing at start of block
        OK(stream.BytesUntilLimit() == 0);
        stream.PopLimit(limit);
    } while (!data.is_end());

    uint16_t server_class_count;
    OK(demo::ReadLittleEndian16(stream, &server_class_count));

    for (uint16_t i = 0; i < server_class_count; ++i)
    {
        BEFORE(Observer, ServerClassCreate);
        std::unique_ptr<ServerClass> server_class = std::make_unique<ServerClass>();
        server_class->deserialize(stream, this->_network.send_tables);
        const ServerClass& reference = *server_class;
        this->_network.server_classes.emplace(server_class->name, std::move(server_class));
        AFTER(ServerClassCreate, reference);
    }
}

SIMULATION(void, advance_packet_class_info, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_set_pause, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_create_string_table, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_update_string_table, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_voice_initialization, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_voice_data, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_print, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_sounds, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_set_view, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_fix_angle, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_crosshair_angle, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_bsp_decal, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_split_screen, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_user_message, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_entity_message, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_game_event, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_packet_entities, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_temporary_entities, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_prefetch, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_menu, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_game_event_list, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_get_console_variable_value, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_paintmap_data, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_command_key_values, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_encrypted_data, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_hltv_replay, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_broadcast_command, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_player_avatar_data, CodedInputStream& stream) PACKET_SKIP()

SIMULATION(void, advance_packet_unknown, CodedInputStream& stream, int32_t command)
{
    throw GameError("unrecognized message " + std::string(demo::describe_net_message(command)));
}

}
