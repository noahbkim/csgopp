#pragma once

#include <string>
#include <absl/container/flat_hash_map.h>
#include <google/protobuf/io/coded_stream.h>

#include "common/lookup.h"
#include "common/bits.h"
#include "common/interface.h"
#include "common/ring.h"
#include "common/database.h"
#include "demo.h"
#include "client/data_table.h"
#include "client/server_class.h"
#include "client/string_table.h"
#include "client/entity.h"
#include "netmessages.pb.h"

#define LOCAL(EVENT) _event_##EVENT
#define BEFORE(OBSERVER, EVENT, ...) typename OBSERVER::EVENT LOCAL(EVENT)(*this, __VA_ARGS__);
#define AFTER(EVENT, ...) LOCAL(EVENT).handle(*this, __VA_ARGS__);

/// Most notably, this namespace defines the `ClientObserverBase` and
/// `Client`, which together form the basis of csgopp's demo parsing and
/// game client framework.
///
/// @brief Demo parsing and game client.
namespace csgopp::client
{

using google::protobuf::io::CodedInputStream;
using csgopp::common::bits::BitStream;
using csgopp::common::ring::Ring;
using csgopp::error::GameError;
using csgopp::client::data_table::DataTable;
using csgopp::client::server_class::ServerClass;
using csgopp::client::string_table::StringTable;
using csgopp::client::entity::Entity;

constexpr size_t MAX_EDICT_BITS = 11;
constexpr size_t ENTITY_HANDLE_INDEX_MASK = (1 << MAX_EDICT_BITS) - 1;
constexpr size_t ENTITY_HANDLE_SERIAL_NUMBER_BITS = 10;
constexpr size_t ENTITY_HANDLE_BITS = MAX_EDICT_BITS + ENTITY_HANDLE_SERIAL_NUMBER_BITS;
constexpr size_t INVALID_ENTITY_HANDLE = (1 << ENTITY_HANDLE_BITS) - 1;

// Forward declaration for use in `ClientObserverBase`.
template<typename Observer>
class Client;

/// \brief This class serves as a base-class for `Client` observers.
///
/// In order to efficiently handle events emitted by the client, we
/// template an overarching observer and its various nested event observers
/// directly into the client.
///
/// This class provides empty implementations for every event observer used by
/// the client. It therefore serves as a convenient base class for custom
/// observers that might not need to explicitly implement every event--the
/// client just instantiates `Observer::EventObserver`, so if the subclass
/// provides no override, the base's is used.
///
/// An example observer that counts the frame in a demo as well as the
/// frequency of each frame command might look something like the following:
///
/// \code
/// struct StructureObserver : public ClientObserverBase<StructureObserver>
/// {
///     size_t frame_count = 0;
///     std::map<csgopp::demo::Command::Type, size_t> commands;
///
///     struct FrameObserver final : public ClientObserverBase::FrameObserver
///     {
///         using ClientObserverBase::FrameObserver::FrameObserver;
///
///         void handle(Client& client, csgopp::demo::Command::Type command) override
///         {
///             client.observer.frame_count += 1;
///             client.observer.commands[command] += 1;
///         }
///     };
/// };
/// \endcode
///
/// \tparam ClientObserver should be the subclass type, e.g.
///     `struct CustomObserver : ClientObserverBase<CustomObserver> {}`.
///     This is necessary to properly type the `Client` reference that's
///     passed to both the constructor and `handle` method.
template<typename ClientObserver>
struct ClientObserverBase
{
    /// Convenience; avoids having to rewrite the full type.
    using Client = Client<ClientObserver>;

    /// Client also receives a reference to the client for setup.
    explicit ClientObserverBase(Client& client) {}

    /// Called by the default frame observer.
    virtual void on_frame(Client& client, demo::Command::Type command) {}

    /// \brief This event is emitted when a DEMO frame is parsed.
    struct FrameObserver
    {
        FrameObserver() = default;
        explicit FrameObserver(Client& client) {}
        virtual void handle(Client& client, demo::Command::Type command)
        {
            client.observer.on_frame(client, command);
        }
    };

    /// Called by the default packet observer.
    virtual void on_packet(Client& client, int32_t type) {}

    /// \brief This event is emitted when a game packet is parsed.
    struct PacketObserver
    {
        PacketObserver() = default;
        explicit PacketObserver(Client& client) {}
        virtual void handle(Client& client, int32_t type)
        {
            client.observer.on_packet(client, type);
        }
    };

    /// Called by the default data table creation observer.
    virtual void on_data_table_creation(Client& client, const DataTable* data_table) {}

    /// \brief This event is emitted when a network data table is created.
    struct DataTableCreationObserver
    {
        DataTableCreationObserver() = default;
        explicit DataTableCreationObserver(Client& client) {}
        virtual void handle(Client& client, const DataTable* data_table)
        {
            client.observer.on_data_table_creation(client, data_table);
        }
    };

    /// Called by the default server class creation observer.
    virtual void on_server_class_creation(Client& client, const ServerClass* server_class) {}

    /// \brief This event is emitted when a network server class is created.
    struct ServerClassCreationObserver
    {
        ServerClassCreationObserver() = default;
        explicit ServerClassCreationObserver(Client& client) {}
        virtual void handle(Client& client, const ServerClass* server_class)
        {
            client.observer.on_server_class_creation(client, server_class);
        }
    };

    /// Called by the default server class creation observer.
    virtual void on_string_table_creation(Client& client, const StringTable* string_table) {}

    /// \brief This event is emitted when a network string table is created.
    struct StringTableCreationObserver
    {
        StringTableCreationObserver() = default;
        explicit StringTableCreationObserver(Client& client) {}
        virtual void handle(Client& client, const StringTable* string_table)
        {
            client.observer.on_string_table_creation(client, string_table);
        }
    };

    /// Called by the default server class update observer.
    virtual void on_string_table_update(Client& client, const StringTable* string_table) {}

    /// \brief This event is emitted when a network string table is created.
    struct StringTableUpdateObserver
    {
        StringTableUpdateObserver() = default;
        explicit StringTableUpdateObserver(Client& client, const StringTable* string_table) {}
        virtual void handle(Client& client, const StringTable* string_table)
        {
            client.observer.on_string_table_update(client, string_table);
        }
    };
};

/// \brief The core DEMO parser and game client.
///
/// This is where the magic happens! The Client object is responsible for
/// consuming frames from the input stream, parsing them, and mutating the
/// client state correspondingly.
///
/// \tparam Observer The templated observer is instantiated as a class member
///     by the constructor and its nested event observers are compiled directly
///     into the client runtime where events are emitted. While this system
///     incurs longer compile times, it's also zero-cost. The optimizer should
///     have no problem remove empty observer calls.
template<typename Observer>
class Client
{
public:
    using DataTableDatabase = csgopp::client::data_table::DataTableDatabase;
    using ServerClassDatabase = csgopp::client::server_class::ServerClassDatabase;
    using StringTableDatabase = csgopp::client::string_table::StringTableDatabase;
    using EntityDatabase = csgopp::client::entity::EntityDatabase;

    /// Instantiate a new client with an observer.
    ///
    /// Arguments are passed directly to the observer preceded by a reference
    /// to the client once the header has been parser.
    ///
    /// \tparam Args observer constructor arguments.
    /// \param stream a stream to read a DEMO header from.
    /// \param args observer constructor arguments.
    template<typename... Args>
    explicit Client(CodedInputStream& stream, Args... args);

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

    /// Interface
    [[nodiscard]] const demo::Header& header() { return this->_header; }
    [[nodiscard]] uint32_t cursor() { return this->_cursor; }
    [[nodiscard]] uint32_t tick() { return this->_tick; }

    Observer observer;

protected:
    demo::Header _header;
    uint32_t _cursor{0};
    uint32_t _tick{0};
    DataTableDatabase _data_tables;
    ServerClassDatabase _server_classes;
    StringTableDatabase _string_tables;
    EntityDatabase _entities;

    /// Helpers
    void create_data_tables(CodedInputStream& stream);
    void populate_string_table(StringTable* string_table, const std::string& blob, int32_t count);
    Entity* create_entity(Entity::Id index, BitStream& data);
    void update_entity(Entity* entity, BitStream& data);
    void delete_entity(Entity::Id index);

private:
    /// Cast this as a const reference for template constructors.
    ///
    /// \todo Figure out how to avoid this.
    [[nodiscard]] inline Client& self() { return *this; }
};

template<typename Observer>
template<typename... Args>
Client<Observer>::Client(CodedInputStream& stream, Args... args)
    : _header(stream)
    , observer(self(), args...) {}

template<typename Observer>
bool Client<Observer>::advance(CodedInputStream& stream)
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
void Client<Observer>::advance_packets(CodedInputStream& stream)
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
void Client<Observer>::advance_packet(CodedInputStream& stream)
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
void Client<Observer>::advance_console_command(CodedInputStream& stream)
{
    uint32_t size;
    OK(stream.ReadLittleEndian32(&size));
    OK(stream.Skip(size));
}

template<typename Observer>
void Client<Observer>::advance_user_command(CodedInputStream& stream)
{
    OK(stream.Skip(4));
    uint32_t size;
    OK(stream.ReadLittleEndian32(&size));
    OK(stream.Skip(size));
}

template<typename Observer>
void Client<Observer>::advance_data_tables(CodedInputStream& stream)
{
    uint32_t size;
    OK(stream.ReadLittleEndian32(&size));
    CodedInputStream::Limit limit = stream.PushLimit(size);

    this->create_data_tables(stream);

    OK(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

/// \see https://github.com/markus-wa/demoinfocs-golang/blob/50f55785b7a0ba89164662a000e00cd55969f7ae/pkg/demoinfocs/stringtables.go#L66
/// \todo actually test this method
template<typename Observer>
void Client<Observer>::advance_string_tables(CodedInputStream& stream)
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

        StringTable* string_table = new StringTable(std::move(name), entry_count);

        for (uint16_t j = 0; j < entry_count; ++j)
        {
            StringTable::Entry* entry = new StringTable::Entry();
            OK(data.read_string(entry->string));
            string_table->entries.at(j) = entry;

            uint8_t has_data;
            OK(data.read(&has_data, 1));
            if (has_data)
            {
                uint16_t data_size;
                OK(data.read(&data_size, 16));

                entry->data.resize(data_size);
                for (uint16_t k = 0; k < data_size; ++k)
                {
                    OK(data.read(&entry->data.at(j), 8));
                }
            }
        }

        AFTER(StringTableCreationObserver, string_table);
    }

    OK(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

template<typename Observer>
void Client<Observer>::advance_custom_data(CodedInputStream& stream)
{
    throw GameError("encountered unexpected CUSTOM_DATA event!");
}

template<typename Observer>
bool Client<Observer>::advance_unknown(CodedInputStream& stream, char command)
{
    throw GameError("encountered unknown command " + std::to_string(command));
}

inline void advance_packet_skip(CodedInputStream& stream)
{
    uint32_t size;
    OK(stream.ReadVarint32(&size));
    OK(stream.Skip(static_cast<int32_t>(size)));
}

template<typename Observer>
void Client<Observer>::advance_packet_nop(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_disconnect(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_file(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_split_screen_user(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_tick(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_string_command(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_set_console_variable(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_sign_on_state(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_server_info(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

static inline DataTable::Int32Property* create_data_table_int32_property(
    DataTable* data_table,
    const csgo::message::net::CSVCMsg_SendTable_sendprop_t& property_data)
{
    if (property_data.flags() & csgopp::client::data_table::property::PropertyFlags::UNSIGNED)
    {
        return new DataTable::UnsignedInt32Property(data_table, property_data);
    }
    else
    {
        return new DataTable::SignedInt32Property(data_table, property_data);
    }
}

static inline DataTable::Int64Property* create_data_table_int64_property(
    DataTable* data_table,
    const csgo::message::net::CSVCMsg_SendTable_sendprop_t& property_data)
{
    if (property_data.flags() & csgopp::client::data_table::property::PropertyFlags::UNSIGNED)
    {
        return new DataTable::UnsignedInt64Property(data_table, property_data);
    }
    else
    {
        return new DataTable::SignedInt64Property(data_table, property_data);
    }
}

static inline DataTable::Property* create_data_table_property(
    DataTable::Property::Type::T type,
    DataTable* data_table,
    const csgo::message::net::CSVCMsg_SendTable_sendprop_t& property_data)
{
    switch (type)
    {
        using Type = DataTable::Property::Type;
        case Type::INT32:
            return create_data_table_int32_property(data_table, property_data);
        case Type::FLOAT:
            return new DataTable::FloatProperty(data_table, property_data);
        case Type::VECTOR3:
            return new DataTable::Vector3Property(data_table, property_data);
        case Type::VECTOR2:
            return new DataTable::Vector2Property(data_table, property_data);
        case Type::STRING:
            return new DataTable::StringProperty(data_table, property_data);
        case Type::ARRAY:
            return new DataTable::ArrayProperty(data_table, property_data);
        case Type::DATA_TABLE:
            return new DataTable::DataTableProperty(data_table, property_data);
        case Type::INT64:
            return create_data_table_int64_property(data_table, property_data);
        default:
            throw csgopp::error::GameError("unreachable");
    }
}

template<typename Observer>
void Client<Observer>::create_data_tables(CodedInputStream& stream)
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
            DataTable* data_table = new DataTable(data);
            using csgo::message::net::CSVCMsg_SendTable_sendprop_t;
            for (const CSVCMsg_SendTable_sendprop_t& property_data : data.props())
            {
                DataTable::Property* property = create_data_table_property(
                    property_data.type(),
                    data_table,
                    property_data);
                data_table->properties.emplace(property);
            }
            this->_data_tables.emplace(data_table);
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
        ServerClass* server_class = new ServerClass();
        OK(csgopp::demo::ReadLittleEndian16(stream, &server_class->id));
        OK(csgopp::demo::ReadCStyleString(stream, &server_class->name));

        std::string data_table_name;
        OK(csgopp::demo::ReadCStyleString(stream, &data_table_name));
        server_class->data_table = this->_data_tables.at(data_table_name);
        server_class->data_table->server_class = server_class;
        temporary_server_class_lookup.emplace(server_class->name, server_class);
    }

    for (auto [_, server_class] : temporary_server_class_lookup)
    {
        for (DataTable::Property* property : server_class->data_table->properties)
        {
            if (property->type == DataTable::Property::Type::DATA_TABLE && property->name == "baseclass")
            {
                const auto& actual = property->as<const DataTable::DataTableProperty>();
                DataTable* base_data_table = this->_data_tables.at(actual.key);
                server_class->base_classes.push_back(base_data_table->server_class);
            }
            else
            {
                server_class->properties.emplace(property);
            }
        }

        BEFORE(Observer, ServerClassCreationObserver);
        this->_server_classes.emplace(server_class);
        AFTER(ServerClassCreationObserver, server_class);
    }
}

template<typename Observer>
void Client<Observer>::advance_packet_send_table(CodedInputStream& stream)
{
    CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
    OK(limit != 0);

    this->create_data_tables(stream);

    OK(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

template<typename Observer>
void Client<Observer>::advance_packet_class_info(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_set_pause(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

/// \see https://github.com/markus-wa/demoinfocs-golang/blob/50f55785b7a0ba89164662a000e00cd55969f7ae/pkg/demoinfocs/stringtables.go#L163
template<typename Observer>
void Client<Observer>::advance_packet_create_string_table(CodedInputStream& stream)
{
    CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
    OK(limit > 0);

    csgo::message::net::CSVCMsg_CreateStringTable data;
    OK(data.ParseFromCodedStream(&stream));

    BEFORE(Observer, StringTableCreationObserver);
    StringTable* string_table = new StringTable(data);
    this->populate_string_table(string_table, data.string_data(), data.num_entries());
    this->_string_tables.emplace(string_table);
    AFTER(StringTableCreationObserver, string_table);

    OK(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

template<typename Observer>
void Client<Observer>::advance_packet_update_string_table(CodedInputStream& stream)
{
    CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
    OK(limit > 0);

    csgo::message::net::CSVCMsg_UpdateStringTable data;
    OK(data.ParseFromCodedStream(&stream));

    size_t index = data.table_id();
    StringTable* string_table = this->_string_tables.at(index);  // TODO revisit if we remove
    ASSERT(string_table != nullptr, "expected a string table at index %zd", index);

    BEFORE(Observer, StringTableUpdateObserver, string_table);
    this->populate_string_table(string_table, data.string_data(), data.num_changed_entries());
    AFTER(StringTableUpdateObserver, string_table);

    OK(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

template<typename Observer>
void Client<Observer>::populate_string_table(StringTable* string_table, const std::string& blob, int32_t count)
{
    BitStream string_data(blob);
    uint8_t verification_bit;
    string_data.read(&verification_bit, 1);
    OK(verification_bit == 0);

    Ring<std::string_view, 32> string_table_entry_history;  // 32 appears to be constant

    size_t index_size = csgopp::common::bits::width(string_table->capacity);
    StringTable::Index auto_increment = 0;
    for (int32_t i = 0; i < count; ++i)
    {
        uint8_t use_auto_increment;
        OK(string_data.read(&use_auto_increment, 1));
        if (!use_auto_increment)
        {
            OK(string_data.read(&auto_increment, index_size));
        }

        // Append
        StringTable::Entry* entry = nullptr;
        if (auto_increment == string_table->entries.size())
        {
            entry = new StringTable::Entry();
            string_table->entries.emplace(entry);
        }
        else
        {
            entry = string_table->entries.at(auto_increment);
            if (entry == nullptr)
            {
                entry = string_table->entries.at(auto_increment) = new StringTable::Entry();
            }
        }

        uint8_t has_string;
        OK(string_data.read(&has_string, 1));
        if (has_string)
        {
            entry->string.clear();

            uint8_t append_to_existing;
            OK(string_data.read(&append_to_existing, 1));
            if (append_to_existing)
            {
                uint8_t history_index;
                OK(string_data.read(&history_index, 5));
                uint8_t bytes_to_copy;
                OK(string_data.read(&bytes_to_copy, 5));
                entry->string.append(string_table_entry_history.at(history_index), 0, bytes_to_copy);
            }

            // Read a c-style string; take until null
            OK(string_data.read_string(entry->string));
        }

        string_table_entry_history.push_back_overwrite(entry->string);
        uint8_t has_data;
        OK(string_data.read(&has_data, 1));
        if (has_data)
        {
            if (string_table->data_fixed)  // < 8 bits
            {
                OK(string_table->data_size_bits <= 8);
                entry->data.push_back(0);
                string_data.read(&entry->data.back(), string_table->data_size_bits);
            }
            else
            {
                uint16_t data_size;
                OK(string_data.read(&data_size, 14));
                entry->data.resize(data_size);
                for (uint16_t j = 0; j < data_size; ++j)
                {
                    OK(string_data.read(&entry->data.at(j), 8));
                }
            }
        }

        auto_increment += 1;
    }
}

template<typename Observer>
void Client<Observer>::advance_packet_voice_initialization(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_voice_data(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_print(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_sounds(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_set_view(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_fix_angle(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_crosshair_angle(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_bsp_decal(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_split_screen(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_user_message(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_entity_message(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_game_event(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_packet_entities(CodedInputStream& stream)
{
    advance_packet_skip(stream);
    return;

    CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
    OK(limit > 0);

    csgo::message::net::CSVCMsg_PacketEntities data;
    OK(data.ParseFromCodedStream(&stream));

    BitStream entity_data(data.entity_data());
    uint32_t auto_increment = 0;
    for (uint32_t i = 0; i < data.updated_entries(); ++i)
    {
        uint32_t auto_increment_skip;
        entity_data.read_compressed_uint32(&auto_increment_skip);
        auto_increment += auto_increment_skip;

        uint8_t command;
        entity_data.read(&command, 2);

        if (command & 0b1)
        {
            this->delete_entity(auto_increment);
        }
        else
        {
            Entity* entity;
            if (command & 0b10)
            {
                entity = this->create_entity(auto_increment, entity_data);
            }
            else
            {
                entity = this->_entities.at(auto_increment);
            }

            this->update_entity(entity, entity_data);
        }

        auto_increment += 1;
    }

    OK(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

template<typename Observer>
Entity* Client<Observer>::create_entity(Entity::Id id, BitStream& data)
{
    size_t server_class_index_size = csgopp::common::bits::width(this->_server_classes.size());
    ServerClass::Id server_class_id;
    OK(data.read(&server_class_id, server_class_index_size));
    ServerClass* server_class = this->_server_classes.at(server_class_id);

    uint16_t serial_number;
    OK(data.read(&serial_number, ENTITY_HANDLE_SERIAL_NUMBER_BITS));

    Entity* entity = new Entity(id, server_class);
    for (DataTable::Property* property : server_class->properties)
    {
//        Entity::Member* member = (property);
        // todo default
//        entity->members.emplace(member);
    }

    // todo emplace entity

    LOG("create entity of type %s", server_class->name.c_str());
    return nullptr;
}

template<typename Observer>
void Client<Observer>::update_entity(Entity* entity, BitStream& data)
{
    uint8_t small_entity_optimization;
    OK(data.read(&small_entity_optimization, 1));

    Entity::Member::Index auto_index = 0;
    while (true)
    {
        Entity::Member::Index jump;
        if (!small_entity_optimization)
        {
            data.read_compressed_uint16(&jump);
        }
        else
        {
            uint8_t use_auto_index;
            OK(data.read(&use_auto_index, 1));
            if (use_auto_index)
            {
                jump = 0;
            }
            else
            {
                OK(data.read(&jump, 3));
            }
        }

        // Update property

        auto_index += jump;
    }
}

template<typename Observer>
void Client<Observer>::delete_entity(Entity::Id index)
{

}

template<typename Observer>
void Client<Observer>::advance_packet_temporary_entities(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_prefetch(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_menu(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_game_event_list(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_get_console_variable_value(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_paintmap_data(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_command_key_values(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_encrypted_data(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_hltv_replay(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_broadcast_command(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_player_avatar_data(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

template<typename Observer>
void Client<Observer>::advance_packet_unknown(CodedInputStream& stream, int32_t command)
{
    throw GameError("unrecognized message " + std::string(demo::describe_net_message(command)));
}

}
