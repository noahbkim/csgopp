#pragma once

#include <string>
#include <absl/container/flat_hash_map.h>
#include <google/protobuf/io/coded_stream.h>

#include "common/lookup.h"
#include "common/bits.h"
#include "common/interface.h"
#include "common/ring.h"
#include "demo.h"
#include "network/data_table.h"
#include "network/server_class.h"
#include "network/string_table.h"
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
using csgopp::common::bits::BitStream;
using csgopp::common::ring::Ring;
using csgopp::error::GameError;
using csgopp::network::DataTable;
using csgopp::network::ServerClass;
using csgopp::network::StringTable;

template<typename Observer>
class Simulation
{
public:
    struct State
    {

    private:
        friend Simulation;
    };

    struct Network
    {

    private:
        friend Simulation;

        std::vector<std::unique_ptr<DataTable>> _data_tables;
        std::vector<std::unique_ptr<DataTable::Property>> _data_table_properties;
        absl::flat_hash_map<std::string_view, DataTable*> _data_tables_by_name;

        std::vector<std::unique_ptr<ServerClass>> _server_classes;
        absl::flat_hash_map<std::string_view, ServerClass*> _server_classes_by_name;
        absl::flat_hash_map<ServerClass::Id, ServerClass*> _server_classes_by_id;

        std::vector<std::unique_ptr<StringTable>> _string_tables;
        absl::flat_hash_map<std::string_view, StringTable*> _string_tables_by_name;

        template<typename... Args> DataTable* allocate_data_table(Args... args);
        template<typename T, typename... Args> DataTable::Property* allocate_data_table_property(Args... args);
        template<typename... Args> DataTable::Property* allocate_data_table_property(
            DataTable::Property::Type::T type,
            Args... args);
        void publish_data_table(DataTable* data_table);

        template<typename... Args> ServerClass* allocate_server_class(Args... args);
        void publish_server_class(ServerClass* server_class);

        template<typename... Args> StringTable* allocate_string_table(Args... args);
        void publish_string_table(StringTable* string_table);
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
    virtual void advance_string_table(CodedInputStream& stream);
    virtual void advance_custom_data(CodedInputStream& stream);
    virtual bool advance_unknown(CodedInputStream& stream, char command);

    GET(header, const&);
    GET(state, const&);
    GET(network, const&);
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
    NOOP(SendTableCreate, Simulation, const DataTable*);
    NOOP(ServerClassCreate, Simulation, const ServerClass*);
    NOOP(StringTableCreate, Simulation, const StringTable*);
};

#define SIMULATION(TYPE, NAME, ...) template<typename Observer> TYPE Simulation<Observer>::NAME(__VA_ARGS__)

template<typename Observer>
template<typename... Args>
DataTable* Simulation<Observer>::Network::allocate_data_table(Args... args)
{
    std::unique_ptr<DataTable> storage = std::make_unique<DataTable>(args...);
    return this->_data_tables.emplace_back(std::move(storage)).get();
}

template<typename Observer>
template<typename T, typename... Args>
DataTable::Property* Simulation<Observer>::Network::allocate_data_table_property(Args... args)
{
    std::unique_ptr<T> storage = std::make_unique<T>(args...);
    return this->_data_table_properties.emplace_back(std::move(storage)).get();
}

template<typename Observer>
template<typename... Args>
DataTable::Property* Simulation<Observer>::Network::allocate_data_table_property(
    DataTable::Property::Type::T type,
    Args... args
) {
    switch (type)
    {
        using Type = DataTable::Property::Type;
        case Type::INT32:
            return this->template allocate_data_table_property<DataTable::Int32Property>(args...);
        case Type::FLOAT:
            return this->template allocate_data_table_property<DataTable::FloatProperty>(args...);
        case Type::VECTOR3:
            return this->template allocate_data_table_property<DataTable::Vector3Property>(args...);
        case Type::VECTOR2:
            return this->template allocate_data_table_property<DataTable::Vector2Property>(args...);
        case Type::STRING:
            return this->template allocate_data_table_property<DataTable::StringProperty>(args...);
        case Type::ARRAY:
            return this->template allocate_data_table_property<DataTable::ArrayProperty>(args...);
        case Type::DATA_TABLE:
            return this->template allocate_data_table_property<DataTable::DataTableProperty>(args...);
        case Type::INT64:
            return this->template allocate_data_table_property<DataTable::Int64Property>(args...);
        default:
            throw csgopp::error::GameError("unreachable");
    }
}

template<typename Observer>
void Simulation<Observer>::Network::publish_data_table(DataTable* data_table)
{
    this->_data_tables_by_name.emplace(data_table->name, data_table);
}

template<typename Observer>
template<typename... Args>
ServerClass* Simulation<Observer>::Network::allocate_server_class(Args... args)
{
    std::unique_ptr<ServerClass> storage = std::make_unique<ServerClass>(args...);
    return this->_server_classes.emplace_back(std::move(storage)).get();
}

template<typename Observer>
void Simulation<Observer>::Network::publish_server_class(ServerClass* server_class)
{
    this->_server_classes_by_name.emplace(server_class->name, server_class);
    this->_server_classes_by_id.emplace(server_class->id, server_class);
}

template<typename Observer>
template<typename... Args>
StringTable* Simulation<Observer>::Network::allocate_string_table(Args... args)
{
    std::unique_ptr<StringTable> storage = std::make_unique<StringTable>(args...);
    return this->_string_tables.emplace_back(std::move(storage)).get();
}

template<typename Observer>
void Simulation<Observer>::Network::publish_string_table(StringTable* string_table)
{
    this->_string_tables_by_name.emplace(string_table->name, string_table);
}

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

SIMULATION(void, advance_packet, CodedInputStream& stream)
{
    BEFORE(Observer, Packet);
    uint32_t command = stream.ReadTag();

    switch (command)
    {
        using namespace csgo::message::net;
        case NET_Messages::net_NOP: this->advance_packet_nop(stream); break;
        case NET_Messages::net_Disconnect: this->advance_packet_disconnect(stream); break;
        case NET_Messages::net_File: this->advance_packet_file(stream); break;
        case NET_Messages::net_SplitScreenUser: this->advance_packet_split_screen_user(stream); break;
        case NET_Messages::net_Tick: this->advance_packet_tick(stream); break;
        case NET_Messages::net_StringCmd: this->advance_packet_string_command(stream); break;
        case NET_Messages::net_SetConVar: this->advance_packet_set_console_variable(stream); break;
        case NET_Messages::net_SignonState: this->advance_packet_sign_on_state(stream); break;
        case SVC_Messages::svc_ServerInfo: this->advance_packet_server_info(stream); break;
        case SVC_Messages::svc_SendTable: this->advance_packet_send_table(stream); break;
        case SVC_Messages::svc_ClassInfo: this->advance_packet_class_info(stream); break;
        case SVC_Messages::svc_SetPause: this->advance_packet_set_pause(stream); break;
        case SVC_Messages::svc_CreateStringTable: this->advance_packet_create_string_table(stream); break;
        case SVC_Messages::svc_UpdateStringTable: this->advance_packet_update_string_table(stream); break;
        case SVC_Messages::svc_VoiceInit: this->advance_packet_voice_initialization(stream); break;
        case SVC_Messages::svc_VoiceData: this->advance_packet_voice_data(stream); break;
        case SVC_Messages::svc_Print: this->advance_packet_print(stream); break;
        case SVC_Messages::svc_Sounds: this->advance_packet_sounds(stream); break;
        case SVC_Messages::svc_SetView: this->advance_packet_set_view(stream); break;
        case SVC_Messages::svc_FixAngle: this->advance_packet_fix_angle(stream); break;
        case SVC_Messages::svc_CrosshairAngle: this->advance_packet_crosshair_angle(stream); break;
        case SVC_Messages::svc_BSPDecal: this->advance_packet_bsp_decal(stream); break;
        case SVC_Messages::svc_SplitScreen: this->advance_packet_split_screen(stream); break;
        case SVC_Messages::svc_UserMessage: this->advance_packet_user_message(stream); break;
        case SVC_Messages::svc_EntityMessage: this->advance_packet_entity_message(stream); break;
        case SVC_Messages::svc_GameEvent: this->advance_packet_game_event(stream); break;
        case SVC_Messages::svc_PacketEntities: this->advance_packet_packet_entities(stream); break;
        case SVC_Messages::svc_TempEntities: this->advance_packet_temporary_entities(stream); break;
        case SVC_Messages::svc_Prefetch: this->advance_packet_prefetch(stream); break;
        case SVC_Messages::svc_Menu: this->advance_packet_menu(stream); break;
        case SVC_Messages::svc_GameEventList: this->advance_packet_game_event_list(stream); break;
        case SVC_Messages::svc_GetCvarValue: this->advance_packet_get_console_variable_value(stream); break;
        case SVC_Messages::svc_PaintmapData: this->advance_packet_paintmap_data(stream); break;
        case SVC_Messages::svc_CmdKeyValues: this->advance_packet_command_key_values(stream); break;
        case SVC_Messages::svc_EncryptedData: this->advance_packet_encrypted_data(stream); break;
        case SVC_Messages::svc_HltvReplay: this->advance_packet_hltv_replay(stream); break;
        case SVC_Messages::svc_Broadcast_Command: this->advance_packet_broadcast_command(stream); break;
        case NET_Messages::net_PlayerAvatarData: this->advance_packet_player_avatar_data(stream); break;
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
    CodedInputStream::Limit limit = stream.PushLimit(size);

    uint8_t count;
    stream.ReadRaw(&count, 1);
    for (uint8_t i = 0; i < count; ++i)
    {
        this->advance_string_table(stream);
    }

    OK(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

SIMULATION(void, advance_string_table, CodedInputStream& stream)
{
    std::string name;
    OK(demo::ReadCStyleString(stream, &name));

    uint16_t count;
    OK(demo::ReadLittleEndian16(stream, &count));

    printf("%d\n", stream.CurrentPosition());

//    for (uint16_t i = 0; i < count; ++i)
//    {
//
//    }

    throw GameError("stop");
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
        // This code isn't encapsulated because the final DataTable, which returns true from data.is_end(), is always
        // empty besides that flag; we don't want to bother initializing a bunch of resources for that.
        OK(stream.ExpectTag(csgo::message::net::SVC_Messages::svc_SendTable));
        CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
        OK(limit > 0);

        // Break if we're at the end of the data_table.
        data.ParseFromCodedStream(&stream);

        // Actually do event handling if we're not at the terminator.
        if (!data.is_end())
        {
            BEFORE(Observer, SendTableCreate);
            DataTable* data_table = this->_network.allocate_data_table(data);
            using csgo::message::net::CSVCMsg_SendTable_sendprop_t;
            for (const CSVCMsg_SendTable_sendprop_t& property_data : data.props())
            {
                DataTable::Property* property = this->_network.allocate_data_table_property(
                    property_data.type(),
                    data_table,
                    property_data);
                data_table->properties.emplace(property->name, property);
            }
            this->_network.publish_data_table(data_table);
            AFTER(SendTableCreate, data_table);
        }

        // Related to inline parsing at start of block
        OK(stream.BytesUntilLimit() == 0);
        stream.PopLimit(limit);
    } while (!data.is_end());

    uint16_t server_class_count;
    OK(demo::ReadLittleEndian16(stream, &server_class_count));
    absl::flat_hash_map<std::string_view, ServerClass*> temporary_server_class_lookup;
    for (uint16_t i = 0; i < server_class_count; ++i)
    {
        ServerClass* server_class = this->_network.allocate_server_class();
        OK(csgopp::demo::ReadLittleEndian16(stream, &server_class->id));
        OK(csgopp::demo::ReadCStyleString(stream, &server_class->name));

        std::string data_table_name;
        OK(csgopp::demo::ReadCStyleString(stream, &data_table_name));
        server_class->data_table = this->_network._data_tables_by_name.at(data_table_name);
        server_class->data_table->server_class = server_class;
        temporary_server_class_lookup.emplace(server_class->name, server_class);
    }

    for (auto [_, server_class] : temporary_server_class_lookup)
    {
        for (const auto& [name, property] : server_class->data_table->properties)
        {
            if (property->type == DataTable::Property::Type::DATA_TABLE && name == "baseclass")
            {
                const auto& actual = property->as<const DataTable::DataTableProperty>();
                DataTable* base_data_table = this->_network._data_tables_by_name.at(actual.key);
                server_class->base_classes.push_back(base_data_table->server_class);
            }
            else
            {
                server_class->properties.emplace(name, property);
            }
        }

        BEFORE(Observer, ServerClassCreate);
        this->_network.publish_server_class(server_class);
        AFTER(ServerClassCreate, server_class);
    }
}

SIMULATION(void, advance_packet_class_info, CodedInputStream& stream) PACKET_SKIP()
SIMULATION(void, advance_packet_set_pause, CodedInputStream& stream) PACKET_SKIP()

SIMULATION(void, advance_packet_create_string_table, CodedInputStream& stream)
{
    CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
    OK(limit > 0);

    BEFORE(Observer, StringTableCreate);
    csgo::message::net::CSVCMsg_CreateStringTable data;
    OK(data.ParseFromCodedStream(&stream));

    StringTable* string_table = this->_network.allocate_string_table(data);
    BitStream string_data(data.string_data());

    uint8_t verification_bit;
    string_data.read(&verification_bit, 1);
    OK(verification_bit == 0);

    Ring<std::string_view, 32> string_table_entry_history;  // 31 appears to be constant

    size_t index_size = csgopp::common::bits::width(data.max_entries());
    StringTable::Index auto_increment = 0;

    for (int32_t i = 0; i < data.num_entries(); ++i)
    {
        uint8_t use_auto_increment;
        OK(string_data.read(&use_auto_increment, 1));
        if (!use_auto_increment)
        {
            OK(string_data.read(&auto_increment, index_size));
        }

        std::string string;

        uint8_t has_string;
        OK(string_data.read(&has_string, 1));
        if (has_string)
        {
            uint8_t append;
            OK(string_data.read(&append, 1));
            if (append)
            {
                uint8_t history_index;
                OK(string_data.read(&history_index, 5));
                uint8_t bytes_to_copy;
                OK(string_data.read(&bytes_to_copy, 5));
                string.append(string_table_entry_history.at(history_index), 0, bytes_to_copy);
            }

            // Read a c-style string; take until null
            do
            {
                string.push_back(0);
                OK(string_data.read(&string.back(), 8));
            } while (string.back() != 0);
            string.pop_back();
        }

        string_table_entry_history.push_back_overwrite(string);
        uint8_t has_user_data;
        OK(string_data.read(&has_user_data, 1));
        if (has_user_data)
        {
            std::vector<uint8_t> user_data;
            if (data.user_data_fixed_size())  // < 8 bits
            {
                OK(data.user_data_size_bits() <= 8);
                user_data.push_back(0);
                string_data.read(&user_data.back(), data.user_data_size_bits());
            }
            else
            {
                uint16_t user_data_size;
                OK(string_data.read(&user_data_size, 14));
                user_data.resize(user_data_size);
                for (uint16_t j = 0; j < user_data_size; ++j)
                {
                    OK(string_data.read(&user_data.at(j), 8));
                }
            }
        }

        string_table->strings.emplace(auto_increment, std::move(string));
        auto_increment += 1;
    }

    this->_network.publish_string_table(string_table);
    AFTER(StringTableCreate, string_table);

    OK(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

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
