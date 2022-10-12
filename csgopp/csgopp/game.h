#pragma once

#include <string>
#include <absl/container/flat_hash_map.h>
#include <google/protobuf/io/coded_stream.h>

#include "common/lookup.h"
#include "common/bits.h"
#include "common/interface.h"
#include "common/ring.h"
#include "demo.h"
#include "network.h"
#include "netmessages.pb.h"

#define LOCAL(EVENT) _event_##EVENT
#define BEFORE(OBSERVER, EVENT, ...) typename OBSERVER::EVENT LOCAL(EVENT)(*this, __VA_ARGS__);
#define AFTER(EVENT, ...) LOCAL(EVENT).handle(*this, __VA_ARGS__);

/// Most notably, this namespace defines the `SimulationObserverBase` and
/// `Simulation`, which together form the basis of csgopp's demo parsing and
/// game simulation framework.
///
/// @brief Demo parsing and game simulation.
namespace csgopp::game
{

using google::protobuf::io::CodedInputStream;
using csgopp::common::bits::BitStream;
using csgopp::common::ring::Ring;
using csgopp::error::GameError;
using csgopp::network::DataTable;
using csgopp::network::ServerClass;
using csgopp::network::StringTable;

// Forward declaration for use in `SimulationObserverBase`.
template<typename Observer>
class Simulation;

/// \brief This class serves as a base-class for `Simulation` observers.
///
/// In order to efficiently handle events emitted by the simulation, we
/// template an overarching observer and its various nested event observers
/// directly into the simulation.
///
/// This class provides empty implementations for every event observer used by
/// the simulation. It therefore serves as a convenient base class for custom
/// observers that might not need to explicitly implement every event--the
/// simulation just instantiates `Observer::EventObserver`, so if the subclass
/// provides no override, the base's is used.
///
/// An example observer that counts the frame in a demo as well as the
/// frequency of each frame command might look something like the following:
///
/// \code
/// struct StructureObserver : public SimulationObserverBase<StructureObserver>
/// {
///     size_t frame_count = 0;
///     std::map<csgopp::demo::Command::Type, size_t> commands;
///
///     struct FrameObserver final : public SimulationObserverBase::FrameObserver
///     {
///         using SimulationObserverBase::FrameObserver::FrameObserver;
///
///         void handle(Simulation& simulation, csgopp::demo::Command::Type command) override
///         {
///             simulation.observer.frame_count += 1;
///             simulation.observer.commands[command] += 1;
///         }
///     };
/// };
/// \endcode
///
/// \tparam SimulationObserver should be the subclass type, e.g.
///     `struct CustomObserver : SimulationObserverBase<CustomObserver> {}`.
///     This is necessary to properly type the `Simulation` reference that's
///     passed to both the constructor and `handle` method.
template<typename SimulationObserver>
struct SimulationObserverBase
{
    /// Convenience; avoids having to rewrite the full type.
    using Simulation = Simulation<SimulationObserver>;

    explicit SimulationObserverBase(Simulation& simulation) {}

    /// \brief This event is emitted when a DEMO frame is parsed.
    struct FrameObserver
    {
        explicit FrameObserver(Simulation& simulation) {}
        virtual void handle(Simulation& simulation, demo::Command::Type type) {}
    };

    /// \brief This event is emitted when a game packet is parsed.
    struct PacketObserver
    {
        explicit PacketObserver(Simulation& simulation) {}
        virtual void handle(Simulation& simulation, int32_t packet) {}
    };

    /// \brief This event is emitted when a network data table is created.
    struct DataTableCreationObserver
    {
        explicit DataTableCreationObserver(Simulation& simulation) {}
        virtual void handle(Simulation& simulation, const DataTable* data_table) {}
    };

    /// \brief This event is emitted when a network server class is created.
    struct ServerClassCreationObserver
    {
        explicit ServerClassCreationObserver(Simulation& simulation) {}
        virtual void handle(Simulation& simulation, const ServerClass* server_class) {}
    };

    /// \brief This event is emitted when a network string table is created.
    struct StringTableCreationObserver
    {
        explicit StringTableCreationObserver(Simulation& simulation) {}
        virtual void handle(Simulation& simulation, StringTable&& string_table) {}
    };
};

/// \brief The core DEMO parser and game simulation.
///
/// This is where the magic happens! The Simulation object is responsible for
/// consuming frames from the input stream, parsing them, and mutating the
/// simulation state correspondingly.
///
/// \tparam Observer The templated observer is instantiated as a class member
///     by the constructor and its nested event observers are compiled directly
///     into the simulation runtime where events are emitted. While this system
///     incurs longer compile times, it's also zero-cost. The optimizer should
///     have no problem remove empty observer calls.
template<typename Observer>
class Simulation
{
public:
    using Network = csgopp::network::Network;

    /// Instantiate a new simulation with an observer.
    ///
    /// Arguments are passed directly to the observer preceded by a reference
    /// to the simulation once the header has been parser.
    ///
    /// \tparam Args observer constructor arguments.
    /// \param stream a stream to read a DEMO header from.
    /// \param args observer constructor arguments.
    template<typename... Args>
    explicit Simulation(CodedInputStream& stream, Args... args);

    /// How about now? Is the issue any better?
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

    [[nodiscard]] const demo::Header& header() { return this->_header; }
    [[nodiscard]] const Network& network() { return this->_network; }
    [[nodiscard]] uint32_t cursor() { return this->_cursor; }
    [[nodiscard]] uint32_t tick() { return this->_tick; }

    Observer observer;

protected:
    demo::Header _header;
    Network _network;
    uint32_t _cursor{0};
    uint32_t _tick{0};

private:
    /// Cast this as a const reference for template constructors.
    ///
    /// \todo Figure out how to avoid this.
    [[nodiscard]] inline Simulation& self() { return *this; }
};

template<typename Observer>
template<typename... Args>
Simulation<Observer>::Simulation(CodedInputStream& stream, Args... args)
    : _header(stream)
    , observer(self(), args...) {}

template<typename Observer>
bool Simulation<Observer>::advance(CodedInputStream& stream)
{
    bool ok = true;
    BEFORE(Observer, FrameObserver);

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
    AFTER(FrameObserver, command);
    return ok;
}

template<typename Observer>
void Simulation<Observer>::advance_packets(CodedInputStream& stream)
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

template<typename Observer>
void Simulation<Observer>::advance_packet(CodedInputStream& stream)
{
    BEFORE(Observer, PacketObserver);
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

    AFTER(PacketObserver, command);
}

template<typename Observer>
void Simulation<Observer>::advance_console_command(CodedInputStream& stream)
{
    uint32_t size;
    OK(stream.ReadLittleEndian32(&size));
    OK(stream.Skip(size));
}

template<typename Observer>
void Simulation<Observer>::advance_user_command(CodedInputStream& stream)
{
    OK(stream.Skip(4));
    uint32_t size;
    OK(stream.ReadLittleEndian32(&size));
    OK(stream.Skip(size));
}

template<typename Observer>
void Simulation<Observer>::advance_data_tables(CodedInputStream& stream)
{
    uint32_t size;
    OK(stream.ReadLittleEndian32(&size));
    CodedInputStream::Limit limit = stream.PushLimit(size);

    this->advance_packet_send_table(stream);

    OK(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

/// \see https://github.com/markus-wa/demoinfocs-golang/blob/50f55785b7a0ba89164662a000e00cd55969f7ae/pkg/demoinfocs/stringtables.go#L66
/// \todo actually test this method
template<typename Observer>
void Simulation<Observer>::advance_string_tables(CodedInputStream& stream)
{
    uint32_t size;
    OK(stream.ReadLittleEndian32(&size));
    CodedInputStream::Limit limit = stream.PushLimit(size);

    BitStream data(stream, size);

    uint8_t count;
    OK(data.read(&count, 8));
    for (uint8_t i = 0; i < count; ++i)
    {
        BEFORE(Observer, StringTableCreationObserver);

        std::string name;
        OK(data.read_string(name));
        uint16_t entry_count;
        OK(data.read(&entry_count, 16));

        StringTable string_table(std::move(name), entry_count);

        for (uint16_t j = 0; j < entry_count; ++j)
        {
            StringTable::Entry& entry = string_table.entries.at(j);
            OK(data.read_string(entry.string));

            uint8_t has_data;
            OK(data.read(&has_data, 1));
            if (has_data)
            {
                uint16_t data_size;
                OK(data.read(&data_size, 16));

                entry.data.resize(data_size);
                for (uint16_t k = 0; k < data_size; ++k)
                {
                    OK(data.read(&entry.data.at(j), 8));
                }
            }
        }

        AFTER(StringTableCreationObserver, std::move(string_table));
    }

    OK(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

template<typename Observer>
void Simulation<Observer>::advance_custom_data(CodedInputStream& stream)
{
    throw GameError("encountered unexpected CUSTOM_DATA event!");
}

template<typename Observer>
bool Simulation<Observer>::advance_unknown(CodedInputStream& stream, char command)
{
    throw GameError("encountered unknown command " + std::to_string(command));
}

inline void advance_packet_skip(CodedInputStream& stream)
{
    uint32_t size;
    OK(stream.ReadVarint32(&size));
    OK(stream.Skip(static_cast<int32_t>(size)));
}

template<typename Observer> void Simulation<Observer>::advance_packet_nop(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_disconnect(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_file(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_split_screen_user(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_tick(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_string_command(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_set_console_variable(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_sign_on_state(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_server_info(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Simulation<Observer>::advance_packet_send_table(CodedInputStream& stream)
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
            BEFORE(Observer, DataTableCreationObserver);
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
            AFTER(DataTableCreationObserver, data_table);
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

        BEFORE(Observer, ServerClassCreationObserver);
        this->_network.publish_server_class(server_class);
        AFTER(ServerClassCreationObserver, server_class);
    }
}

template<typename Observer> void Simulation<Observer>::advance_packet_class_info(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_set_pause(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

/// \see https://github.com/markus-wa/demoinfocs-golang/blob/50f55785b7a0ba89164662a000e00cd55969f7ae/pkg/demoinfocs/stringtables.go#L163
template<typename Observer>
void Simulation<Observer>::advance_packet_create_string_table(CodedInputStream& stream)
{
    CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
    OK(limit > 0);

    BEFORE(Observer, StringTableCreationObserver);
    csgo::message::net::CSVCMsg_CreateStringTable data;
    OK(data.ParseFromCodedStream(&stream));

    StringTable string_table(data.name(), data.num_entries());
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

        StringTable::Entry& entry = string_table.entries.at(i);
        entry.index.emplace(auto_increment);

        uint8_t has_string;
        OK(string_data.read(&has_string, 1));
        if (has_string)
        {
            uint8_t append_to_existing;
            OK(string_data.read(&append_to_existing, 1));
            if (append_to_existing)
            {
                uint8_t history_index;
                OK(string_data.read(&history_index, 5));
                uint8_t bytes_to_copy;
                OK(string_data.read(&bytes_to_copy, 5));
                entry.string.append(string_table_entry_history.at(history_index), 0, bytes_to_copy);
            }

            // Read a c-style string; take until null
            OK(string_data.read_string(entry.string));
        }

        string_table_entry_history.push_back_overwrite(entry.string);
        uint8_t has_data;
        OK(string_data.read(&has_data, 1));
        if (has_data)
        {
            if (data.user_data_fixed_size())  // < 8 bits
            {
                OK(data.user_data_size_bits() <= 8);
                entry.data.push_back(0);
                string_data.read(&entry.data.back(), data.user_data_size_bits());
            }
            else
            {
                uint16_t data_size;
                OK(string_data.read(&data_size, 14));
                entry.data.resize(data_size);
                for (uint16_t j = 0; j < data_size; ++j)
                {
                    OK(string_data.read(&entry.data.at(j), 8));
                }
            }
        }

        auto_increment += 1;
    }

    AFTER(StringTableCreationObserver, std::move(string_table));

    OK(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

//https://developer.valvesoftware.com/wiki/Networking_Events_%26_Messages
//https://developer.valvesoftware.com/wiki/Networking_Entities
//https://gist.github.com/jboner/2841832#file-latency-txt-L12
//https://github.com/markus-wa/gobitread/blob/a316e052584a5cb5f7a6a6285d3636d62146b5aa/bitread.go#L155
//https://github.com/markus-wa/demoinfocs-golang/blob/master/pkg/demoinfocs/stringtables.go
//https://github.com/markus-wa/demoinfocs-golang/blob/50f55785b7a0ba89164662a000e00cd55969f7ae/pkg/demoinfocs/parsing.go

template<typename Observer>
void Simulation<Observer>::advance_packet_update_string_table(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_voice_initialization(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_voice_data(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_print(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_sounds(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_set_view(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_fix_angle(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_crosshair_angle(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_bsp_decal(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_split_screen(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_user_message(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_entity_message(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_game_event(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_packet_entities(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_temporary_entities(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_prefetch(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_menu(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_game_event_list(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_get_console_variable_value(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_paintmap_data(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_command_key_values(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_encrypted_data(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_hltv_replay(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_broadcast_command(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer> void Simulation<Observer>::advance_packet_player_avatar_data(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Simulation<Observer>::advance_packet_unknown(CodedInputStream& stream, int32_t command)
{
    throw GameError("unrecognized message " + std::string(demo::describe_net_message(command)));
}

}
