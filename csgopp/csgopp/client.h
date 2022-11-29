#pragma once

#include <string>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <google/protobuf/io/coded_stream.h>

#include "common/macro.h"
#include "common/lookup.h"
#include "common/bits.h"
#include "common/reader.h"
#include "common/ring.h"
#include "common/database.h"
#include "common/control.h"
#include "demo.h"
#include "client/data_table.h"
#include "client/server_class.h"
#include "client/string_table.h"
#include "client/entity.h"
#include "client/game_event.h"
#include "client/user.h"
#include "netmessages.pb.h"

#define LOCAL(EVENT) _event_##EVENT
#define BEFORE(OBSERVER, EVENT, ...) typename OBSERVER::EVENT LOCAL(EVENT)(*this, ## __VA_ARGS__);
#define AFTER(EVENT, ...) LOCAL(EVENT).handle(*this, __VA_ARGS__);


#ifdef NDEBUG
#define DEBUG(STATEMENT)
#define NOTE(FMT, ...)
#define VERIFY(CONDITION, ...) OK(CONDITION)
#else
#define DEBUG(STATEMENT) STATEMENT;
#define NOTE(FMT, ...) fprintf(stderr, "  " FMT "\n", __VA_ARGS__);
#define VERIFY(CONDITION, ...) do \
{ \
    if (!(CONDITION)) \
    { \
        fprintf(stderr, WHERE() "\n  condition: " #CONDITION "\n  cursor: %d\n  tick: %d\n", this->cursor(), this->tick()); \
        __VA_ARGS__; \
        throw csgopp::error::GameError("failed assertion " #CONDITION); \
    } \
} while (false)
#endif

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
using csgopp::common::control::lookup;
using csgopp::common::database::Database;
using csgopp::common::database::DatabaseWithName;
using csgopp::common::reader::ContainerReader;
using csgopp::common::reader::BigEndian;
using csgopp::common::reader::LittleEndian;
using csgopp::common::object::instantiate;
using csgopp::error::GameError;
using csgopp::client::data_table::DataTable;
using csgopp::client::data_table::is_array_index;
using csgopp::client::server_class::ServerClass;
using csgopp::client::string_table::StringTable;
using csgopp::client::entity::EntityType;
using csgopp::client::entity::Entity;
using csgopp::client::game_event::GameEventType;
using csgopp::client::game_event::GameEvent;
using csgopp::client::user::User;

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
///     std::map<csgopp::demo::Command::T, size_t> commands;
///
///     struct FrameObserver final : public ClientObserverBase::FrameObserver
///     {
///         using ClientObserverBase::FrameObserver::FrameObserver;
///
///         void handle(Client& client, csgopp::demo::Command::T command) override
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

    /// Called by the default server class update observer.
    virtual void on_entity_creation(Client& client, const Entity* entity) {}

    /// \brief This event is emitted when a network string table is created.
    struct EntityCreationObserver
    {
        EntityCreationObserver() = default;
        explicit EntityCreationObserver(Client& client, Entity::Id id, const ServerClass* server_class) {}
        virtual void handle(Client& client, const Entity* entity)
        {
            client.observer.on_entity_creation(client, entity);
        }
    };

    /// Called by the default server class update observer.
    virtual void on_entity_update(Client& client, const Entity* entity, const std::vector<uint16_t>& indices) {}

    /// \brief This event is emitted when a network string table is created.
    struct EntityUpdateObserver
    {
        EntityUpdateObserver() = default;
        explicit EntityUpdateObserver(Client& client, const Entity* entity) {}
        virtual void handle(Client& client, const Entity* entity, const std::vector<uint16_t>& indices)
        {
            client.observer.on_entity_update(client, entity, indices);
        }
    };

    /// Called by the default server class update observer.
    virtual void on_entity_deletion(Client& client, const Entity* entity) {}

    /// \brief This event is emitted when a network string table is created.
    struct EntityDeletionObserver
    {
        EntityDeletionObserver() = default;
        explicit EntityDeletionObserver(Client& client, const Entity* entity) {}
        virtual void handle(Client& client, const Entity* entity)
        {
            client.observer.on_entity_creation(client, entity);
        }
    };

    virtual void on_game_event(Client& client, GameEvent& event) {}

    struct GameEventObserver
    {
        GameEventObserver() = default;
        explicit GameEventObserver(Client& client) {}
        virtual void handle(Client& client, GameEvent& game_event)
        {
            client.observer.on_game_event(client, game_event);
        }
    };

    virtual void on_game_event_type_creation(Client& client, const GameEventType* event) {}

    struct GameEventTypeCreationObserver
    {
        GameEventTypeCreationObserver() = default;
        explicit GameEventTypeCreationObserver(Client& client) {}
        virtual void handle(Client& client, const GameEventType* event)
        {
            client.observer.on_game_event_type_creation(client, event);
        }
    };

    virtual void on_user_creation(Client& client, const User* user) {}

    struct UserCreationObserver
    {
        UserCreationObserver() = default;
        explicit UserCreationObserver(Client& client) {}
        virtual void handle(Client& client, const User* user)
        {
            client.observer.on_user_creation(client, user);
        }
    };

    virtual void on_user_update(Client& client, const User* user) {}

    struct UserUpdateObserver
    {
        UserUpdateObserver() = default;
        explicit UserUpdateObserver(Client& client, const User* user) {}
        virtual void handle(Client& client, const User* user)
        {
            client.observer.on_user_update(client, user);
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
    using GameEventTypeDatabase = csgopp::client::game_event::GameEventTypeDatabase;
    using UserDatabase = csgopp::client::user::UserDatabase;

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
    [[nodiscard]] const DataTableDatabase& data_tables() { return this->_data_tables; }
    [[nodiscard]] const ServerClassDatabase& server_classes() { return this->_server_classes; }
    [[nodiscard]] const StringTableDatabase& string_tables() { return this->_string_tables; }
    [[nodiscard]] const EntityDatabase& entities() { return this->_entities; }
    [[nodiscard]] const GameEventTypeDatabase& game_event_types() { return this->_game_event_types; }
    [[nodiscard]] const UserDatabase& users() { return this->_users; }

    Observer observer;

protected:
    demo::Header _header;
    uint32_t _cursor{0};
    uint32_t _tick{0};
    DataTableDatabase _data_tables;
    ServerClassDatabase _server_classes;
    StringTableDatabase _string_tables;
    EntityDatabase _entities;
    GameEventTypeDatabase _game_event_types;
    UserDatabase _users;

    /// Helper data
    std::vector<uint16_t> _update_entity_indices;

    /// Helpers
    void create_data_tables_and_server_classes(CodedInputStream& stream);
    DatabaseWithName<DataTable> create_data_tables(CodedInputStream& stream);
    Database<ServerClass> create_server_classes(CodedInputStream& stream, DatabaseWithName<DataTable>& new_data_tables);

    void populate_string_table(StringTable* string_table, const std::string& blob, int32_t count);

    void _update_entity(Entity* entity, BitStream& stream);
    void create_entity(Entity::Id id, BitStream& stream);
    void update_entity(Entity::Id id, BitStream& stream);
    void delete_entity(Entity::Id id);

    void _update_user(User* user, const std::string& data);
    void update_user(size_t index, const std::string& data);

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
    VERIFY(stream.ReadRaw(&command, 1));
    VERIFY(stream.ReadLittleEndian32(&this->_tick));
    VERIFY(stream.Skip(1));  // player slot

    switch (command)
    {
        case demo::Command::SIGN_ON:
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
            break;
    }

    this->_cursor += 1;
    AFTER(FrameObserver, command);
    return ok;
}

template<typename Observer>
void Client<Observer>::advance_packets(CodedInputStream& stream)
{
    // Arbitrary player data, seems useless
    VERIFY(stream.Skip(152 + 4 + 4));

    uint32_t size;
    VERIFY(stream.ReadLittleEndian32(&size));
    CodedInputStream::Limit limit = stream.PushLimit(size);

    while (stream.BytesUntilLimit() > 0)
    {
        this->advance_packet(stream);
    }

    VERIFY(stream.BytesUntilLimit() == 0);
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
    VERIFY(stream.ReadLittleEndian32(&size));
    VERIFY(stream.Skip(size));
}

template<typename Observer>
void Client<Observer>::advance_user_command(CodedInputStream& stream)
{
    VERIFY(stream.Skip(4));
    uint32_t size;
    VERIFY(stream.ReadLittleEndian32(&size));
    VERIFY(stream.Skip(size));
}

template<typename Observer>
void Client<Observer>::advance_data_tables(CodedInputStream& stream)
{
    uint32_t size;
    VERIFY(stream.ReadLittleEndian32(&size));
    CodedInputStream::Limit limit = stream.PushLimit(size);

    this->create_data_tables_and_server_classes(stream);

    VERIFY(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

/// \see https://github.com/markus-wa/demoinfocs-golang/blob/50f55785b7a0ba89164662a000e00cd55969f7ae/pkg/demoinfocs/stringtables.go#L66
/// \todo actually test this method
template<typename Observer>
void Client<Observer>::advance_string_tables(CodedInputStream& stream)
{
    uint32_t size;
    VERIFY(stream.ReadLittleEndian32(&size));
    CodedInputStream::Limit limit = stream.PushLimit(size);

    DEBUG(int current_position = stream.CurrentPosition());
    char* raw_data = new char[size];
    stream.ReadRaw(raw_data, size);
    BitStream data(raw_data, size);

    uint8_t count;
    VERIFY(data.read(&count, 8));
    for (uint8_t i = 0; i < count; ++i)
    {
        BEFORE(Observer, StringTableCreationObserver);

        std::string name;
        VERIFY(data.read_string(name));
        uint16_t entry_count;
        VERIFY(data.read(&entry_count, 16));

        StringTable* string_table = new StringTable(std::move(name), entry_count);
        for (uint16_t j = 0; j < entry_count; ++j)
        {
            StringTable::Entry* entry = new StringTable::Entry();
            VERIFY(data.read_string(entry->string));
            string_table->entries.emplace(entry);

            uint8_t has_data;
            VERIFY(data.read(&has_data, 1));
            if (has_data)
            {
                uint16_t data_size;
                VERIFY(data.read(&data_size, 16));

                entry->data.resize(data_size);
                for (uint16_t k = 0; k < data_size; ++k)
                {
                    VERIFY(data.read(&entry->data.at(k), 8));
                }
            }
        }

        this->_string_tables.emplace(string_table);
        AFTER(StringTableCreationObserver, std::as_const(string_table));
    }

    VERIFY(stream.BytesUntilLimit() == 0);
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

struct UnboundDataTableProperty
{
    DataTable::DataTableProperty* property;
    std::string name;

    UnboundDataTableProperty(DataTable::DataTableProperty* property, std::string&& name)
        : property(property)
        , name(name)
    {}
};

template<typename Observer>
DatabaseWithName<DataTable> Client<Observer>::create_data_tables(CodedInputStream& stream)
{
    DatabaseWithName<DataTable> new_data_tables;
    std::vector<UnboundDataTableProperty> new_data_table_properties;

    csgo::message::net::CSVCMsg_SendTable data;
    do
    {
        // This code isn't encapsulated because the final DataTable, which returns true from data.is_end(), is
        // always empty besides that flag; we don't want to bother initializing a bunch of resources for that.
        VERIFY(stream.ExpectTag(csgo::message::net::SVC_Messages::svc_SendTable));
        CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
        VERIFY(limit > 0);

        // Break if we're at the end of the origin.
        data.ParseFromCodedStream(&stream);

        // Actually do event handling if we're not at the terminator.
        if (!data.is_end())
        {
            DataTable* data_table = new DataTable(data);
            DataTable::Property* preceding_array_element{nullptr};

            // Do a check to see if our items are all the same type and enumerated
            bool is_coalesced_array{true};
            DataTable::Property* coalesced_array_element{nullptr};
            size_t coalesced_array_index{0};

            using csgo::message::net::CSVCMsg_SendTable_sendprop_t;
            for (CSVCMsg_SendTable_sendprop_t& property_data : *data.mutable_props())
            {
                if (property_data.flags() & DataTable::Property::Flags::EXCLUDE)
                {
                    data_table->excludes.emplace_back(
                        std::move(*property_data.mutable_dt_name()),
                        std::move(*property_data.mutable_var_name()));
                    continue;
                }

                DataTable::Property* property;
                switch (property_data.type())
                {
                    using Type = DataTable::Property::Type;
                    case Type::INT32:
                        property = new DataTable::Int32Property(std::move(property_data));
                        break;
                    case Type::FLOAT:
                        property = new DataTable::FloatProperty(std::move(property_data));
                        break;
                    case Type::VECTOR3:
                        property = new DataTable::Vector3Property(std::move(property_data));
                        break;
                    case Type::VECTOR2:
                        property = new DataTable::Vector2Property(std::move(property_data));
                        break;
                    case Type::STRING:
                        property = new DataTable::StringProperty(std::move(property_data));
                        break;
                    case Type::ARRAY:
                        VERIFY(preceding_array_element != nullptr);
                        property = new DataTable::ArrayProperty(std::move(property_data), preceding_array_element);
                        preceding_array_element = nullptr;
                        break;
                    case Type::DATA_TABLE:
                        property = new_data_table_properties.emplace_back(
                            new DataTable::DataTableProperty(std::move(property_data)),
                            std::string(std::move(*property_data.mutable_dt_name()))).property;
                        break;
                    case Type::INT64:
                        property = new DataTable::Int64Property(std::move(property_data));
                        break;
                    default:
                        throw csgopp::error::GameError("unreachable");
                }

                // Check if we're still optimizing as array
                if (is_coalesced_array)
                {
                    if (coalesced_array_element == nullptr)
                    {
                        coalesced_array_element = property;
                    }

                    if (
                        is_array_index(property->name, coalesced_array_index)
                        && property->equals(coalesced_array_element)
                    ) {
                        coalesced_array_index += 1;
                    }
                    else
                    {
                        is_coalesced_array = false;
                    }
                }

                // If there's an array property, the element_type type always precedes it; don't both adding
                if (property->flags & DataTable::Property::Flags::INSIDE_ARRAY)
                {
                    VERIFY(preceding_array_element == nullptr);
                    preceding_array_element = property;
                }
                else
                {
                    data_table->properties.emplace(property);
                }
            }

            data_table->is_array = data_table->properties.size() > 0 && is_coalesced_array;

            VERIFY(preceding_array_element == nullptr);
            new_data_tables.emplace(data_table);
        }

        // Related to inline parsing at start of block
        VERIFY(stream.BytesUntilLimit() == 0);
        stream.PopLimit(limit);
    } while (!data.is_end());

    // Link data table properties to their data table
    for (const UnboundDataTableProperty& unbound : new_data_table_properties)
    {
        unbound.property->data_table = lookup(
            new_data_tables.by_name,
            this->_data_tables.by_name,
            unbound.name,
            [&unbound]() { return "failed to find referenced data table " + unbound.name; });
    }

    return new_data_tables;
}

template<typename Observer>
Database<ServerClass> Client<Observer>::create_server_classes(
    CodedInputStream& stream,
    DatabaseWithName<DataTable>& new_data_tables
) {
    uint16_t server_class_count;
    VERIFY(demo::ReadLittleEndian16(stream, &server_class_count));
    Database<ServerClass> new_server_classes(server_class_count);

    for (uint16_t i = 0; i < server_class_count; ++i)
    {
        auto* server_class = new ServerClass();
        VERIFY(csgopp::demo::ReadLittleEndian16(stream, &server_class->index));
        VERIFY(csgopp::demo::ReadCStyleString(stream, &server_class->name));

        std::string data_table_name;
        VERIFY(csgopp::demo::ReadCStyleString(stream, &data_table_name));
        server_class->data_table = lookup(
            new_data_tables.by_name,
            this->_data_tables.by_name,
            data_table_name,
            [&data_table_name]() { return "failed to find referenced data table " + data_table_name; });
        server_class->data_table->server_class = server_class;
        new_server_classes.emplace(server_class);
    }

    for (ServerClass* server_class : new_server_classes)
    {
        for (DataTable::Property* property : server_class->data_table->properties)
        {
            if (property->name == "baseclass")
            {
                if (auto* data_table_property = dynamic_cast<DataTable::DataTableProperty*>(property))
                {
                    ASSERT(server_class->base_class == nullptr, "received two base classes for one server class");
                    VERIFY(data_table_property != nullptr);
                    VERIFY(data_table_property->data_table != nullptr);
                    VERIFY(data_table_property->data_table->server_class != nullptr);
                    server_class->base_class = data_table_property->data_table->server_class;
                }
            }
        }
    }

    return new_server_classes;
}

template<typename Observer>
void Client<Observer>::create_data_tables_and_server_classes(CodedInputStream& stream)
{
    DatabaseWithName<DataTable> new_data_tables = this->create_data_tables(stream);
    Database<ServerClass> new_server_classes = this->create_server_classes(stream, new_data_tables);

    // Materialize types
    for (ServerClass* server_class : new_server_classes)
    {
        server_class->data_table->materialize();
    }

    // Now we can emplace and emit
    this->_data_tables.reserve(new_data_tables.size());
    for (DataTable* data_table : new_data_tables)
    {
        BEFORE(Observer, DataTableCreationObserver);
        this->_data_tables.emplace(data_table);
        AFTER(DataTableCreationObserver, std::as_const(data_table));
    }

    this->_server_classes.reserve(new_server_classes.size());
    for (ServerClass* server_class : new_server_classes)
    {
        BEFORE(Observer, ServerClassCreationObserver);
        this->_server_classes.emplace(server_class->index, server_class);
        AFTER(ServerClassCreationObserver, std::as_const(server_class));
    }
}

template<typename Observer>
void Client<Observer>::advance_packet_send_table(CodedInputStream& stream)
{
    CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
    VERIFY(limit != 0);

    this->create_data_tables_and_server_classes(stream);

    VERIFY(stream.BytesUntilLimit() == 0);
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
    VERIFY(limit > 0);

    csgo::message::net::CSVCMsg_CreateStringTable data;
    VERIFY(data.ParseFromCodedStream(&stream));

    BEFORE(Observer, StringTableCreationObserver);
    StringTable* string_table = new StringTable(data);
    this->populate_string_table(string_table, data.string_data(), data.num_entries());
    this->_string_tables.emplace(string_table);
    AFTER(StringTableCreationObserver, std::as_const(string_table));

    VERIFY(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

template<typename Observer>
void Client<Observer>::advance_packet_update_string_table(CodedInputStream& stream)
{
    CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
    VERIFY(limit > 0);

    csgo::message::net::CSVCMsg_UpdateStringTable data;
    VERIFY(data.ParseFromCodedStream(&stream));

    size_t index = data.table_id();
    StringTable* string_table = this->_string_tables.at(index);  // TODO revisit if we remove
    ASSERT(string_table != nullptr, "expected a string table at index %zd", index);

    BEFORE(Observer, StringTableUpdateObserver, string_table);
    this->populate_string_table(string_table, data.string_data(), data.num_changed_entries());
    AFTER(StringTableUpdateObserver, std::as_const(string_table));

    VERIFY(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

template<typename Observer>
void Client<Observer>::populate_string_table(StringTable* string_table, const std::string& blob, int32_t count)
{
    BitStream string_data(blob);
    uint8_t verification_bit;
    string_data.read(&verification_bit, 1);
    VERIFY(verification_bit == 0);

    Ring<std::string_view, 32> string_table_entry_history;  // 32 appears to be constant

    size_t index_size = csgopp::common::bits::width(string_table->capacity);
    StringTable::Index auto_increment = 0;
    for (int32_t i = 0; i < count; ++i)
    {
        uint8_t use_auto_increment;
        VERIFY(string_data.read(&use_auto_increment, 1));
        if (!use_auto_increment)
        {
            VERIFY(string_data.read(&auto_increment, index_size));
        }

        // Append
        StringTable::Entry* entry;
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
        VERIFY(string_data.read(&has_string, 1));
        if (has_string)
        {
            entry->string.clear();

            uint8_t append_to_existing;
            VERIFY(string_data.read(&append_to_existing, 1));
            if (append_to_existing)
            {
                uint8_t history_index;
                VERIFY(string_data.read(&history_index, 5));
                uint8_t bytes_to_copy;
                VERIFY(string_data.read(&bytes_to_copy, 5));
                entry->string.append(string_table_entry_history.at(history_index), 0, bytes_to_copy);
            }

            // Read a c-style string; take until null
            VERIFY(string_data.read_string(entry->string));
        }

        string_table_entry_history.push_back_overwrite(entry->string);
        uint8_t has_data;
        VERIFY(string_data.read(&has_data, 1));
        if (has_data)
        {
            if (string_table->data_fixed)  // < 8 bits
            {
                VERIFY(string_table->data_size_bits <= 8);
                entry->data.push_back(0);
                string_data.read(&entry->data.back(), string_table->data_size_bits);
            }
            else
            {
                uint16_t data_size;
                VERIFY(string_data.read(&data_size, 14));
                entry->data.resize(data_size);
                for (uint16_t j = 0; j < data_size; ++j)
                {
                    VERIFY(string_data.read(&entry->data.at(j), 8));
                }
            }
        }

        if (string_table->name == "userinfo")
        {
            size_t user_index = std::stoull(entry->string);
            this->update_user(user_index, entry->data);
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
    CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
    VERIFY(limit > 0);

    BEFORE(Observer, GameEventObserver);
    csgo::message::net::CSVCMsg_GameEvent data;
    data.ParseFromCodedStream(&stream);

    const GameEventType* game_event_type = this->_game_event_types.at_id(data.eventid());
    GameEvent* game_event = instantiate<GameEventType, GameEvent>(game_event_type);
    game_event->id = data.eventid();
    game_event->name = game_event_type->name;

    for (size_t i = 0; i < data.keys_size(); ++i)
    {
        const GameEventType::Member& member = game_event_type->members.at(i);
        csgo::message::net::CSVCMsg_GameEvent_key_t& key = *data.mutable_keys(i);
        auto* game_event_value_type = dynamic_cast<const game_event::GameEventValueType*>(member.type.get());
        VERIFY(game_event_value_type != nullptr);
        game_event_value_type->update(game_event->address + member.offset, std::move(key));
    }

    AFTER(GameEventObserver, *game_event);
    delete game_event;

    VERIFY(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

/// \sa https://github.com/markus-wa/demoinfocs-golang/blob/1b196ffaaf93c531cdae5897091692e14ead19d2/pkg/demoinfocs/parsing.go#L330
/// \sa https://github.com/markus-wa/demoinfocs-golang/blob/9c61151c71c3821c194f60380cac3777e18e7f6d/pkg/demoinfocs/net_messages.go#L15
template<typename Observer>
void Client<Observer>::advance_packet_packet_entities(CodedInputStream& stream)
{
    CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
    VERIFY(limit > 0);

    DEBUG(int current_position = stream.CurrentPosition());
    csgo::message::net::CSVCMsg_PacketEntities data;
    VERIFY(data.ParseFromCodedStream(&stream));

    BitStream entity_data(data.entity_data());
    Entity::Id entity_id = 0;
    for (uint32_t i = 0; i < data.updated_entries(); ++i, ++entity_id)
    {
        uint32_t auto_increment_skip;
        entity_data.read_compressed_uint32(&auto_increment_skip);
        entity_id += auto_increment_skip;

        uint8_t command;
        entity_data.read(&command, 2);

        if (command & 0b1)
        {
            if (command & 0b10)
            {
                this->delete_entity(entity_id);
            }
        }
        else
        {
            if (command & 0b10)
            {
                this->create_entity(entity_id, entity_data);
            }
            else
            {
                this->update_entity(entity_id, entity_data);
            }
        }
    }

    VERIFY(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

/// \sa https://github.com/markus-wa/demoinfocs-golang/blob/9c61151c71c3821c194f60380cac3777e18e7f6d/pkg/demoinfocs/sendtables/entity.go#L104
template<typename Observer>
void Client<Observer>::_update_entity(Entity* entity, BitStream& stream)
{
    uint8_t small_increment_optimization;
    OK(stream.read(&small_increment_optimization, 1));

    // Keeping this static GREATLY reduces the number of spurious allocations
    this->_update_entity_indices.clear();

    // It's honestly probably more efficient to read through this twice than it is to allocate and make copies
    uint16_t index = 0;
    while (true)
    {
        uint16_t jump;
        if (small_increment_optimization)
        {
            uint8_t use_auto_index;
            VERIFY(stream.read(&use_auto_index, 1));
            if (use_auto_index)
            {
                jump = 0;
            }
            else
            {
                uint8_t is_small_jump;
                VERIFY(stream.read(&is_small_jump, 1));
                if (is_small_jump)
                {
                    VERIFY(stream.read(&jump, 3));
                }
                else
                {
                    VERIFY(stream.read_compressed_uint16(&jump));
                }
            }
        }
        else
        {
            VERIFY(stream.read_compressed_uint16(&jump));
        }

        if (jump == 0xFFF)
        {
            break;
        }
        else
        {
            index += jump;
        }

        this->_update_entity_indices.emplace_back(index);
        index += 1;
    }

    for (uint16_t i : this->_update_entity_indices)
    {
        // Actually update the field
        const entity::Offset& offset = entity->type->prioritized.at(i);
        offset.type->update(entity->address + offset.offset, stream, offset.property);
    }
}

template<typename Observer>
void Client<Observer>::create_entity(Entity::Id id, BitStream& stream)
{
    size_t server_class_index_size = csgopp::common::bits::width(this->_server_classes.size()) + 1;
    ServerClass::Index server_class_id;
    VERIFY(stream.read(&server_class_id, server_class_index_size));
    ServerClass* server_class = this->_server_classes.at(server_class_id);

    uint16_t serial_number;
    VERIFY(stream.read(&serial_number, ENTITY_HANDLE_SERIAL_NUMBER_BITS));

    BEFORE(Observer, EntityCreationObserver, id, std::as_const(server_class));

    VERIFY(server_class->data_table->entity_type != nullptr);
    Entity* entity = instantiate<EntityType, Entity>(server_class->data_table->entity_type.get(), id, server_class);

    // Update from baseline in string table
    VERIFY(this->_string_tables.instance_baseline != nullptr);
    for (StringTable::Entry* entry : this->_string_tables.instance_baseline->entries)
    {
        if (std::stoi(entry->string) == server_class->index)
        {
            BitStream baseline(entry->data);
            this->_update_entity(entity, baseline);
            break;
        }
    }

    // Update from provided data
    this->_update_entity(entity, stream);
    this->_entities.emplace(id, entity);
    AFTER(EntityCreationObserver, std::as_const(entity))
}

/// \sa https://github.com/markus-wa/demoinfocs-golang/blob/9c61151c71c3821c194f60380cac3777e18e7f6d/pkg/demoinfocs/sendtables/entity.go#L104
template<typename Observer>
void Client<Observer>::update_entity(Entity::Id id, BitStream& stream)
{
    Entity* entity = this->_entities.at(id);
    BEFORE(Observer, EntityUpdateObserver, std::as_const(entity));
    this->_update_entity(entity, stream);
    AFTER(EntityUpdateObserver, std::as_const(entity), std::as_const(this->_update_entity_indices));
}

template<typename Observer>
void Client<Observer>::delete_entity(Entity::Id id)
{
    VERIFY(id < this->_entities.size());
    Entity* entity = this->_entities.at(id);
    VERIFY(entity != nullptr);

    BEFORE(Observer, EntityDeletionObserver, std::as_const(entity));
    this->_entities.at(id) = nullptr;
    AFTER(EntityDeletionObserver, std::as_const(entity));
    delete entity;
}

template<typename Observer>
void Client<Observer>::_update_user(User* user, const std::string& data)
{
    ContainerReader<std::string> reader(data);
    user->version = reader.deserialize<uint64_t, BigEndian>();
    user->xuid = reader.deserialize<uint64_t, BigEndian>();
    user->name = reader.terminated(128);
    user->id = reader.deserialize<int32_t, BigEndian>();
    user->guid = reader.terminated(33);
    user->friends_id = reader.deserialize<uint32_t, BigEndian>();
    user->friends_name = reader.terminated(128);
    user->is_fake = reader.deserialize<uint8_t>() != 0;
    user->is_hltv = reader.deserialize<uint8_t>() != 0;
    user->custom_files[0] = reader.deserialize<uint32_t, LittleEndian>();
    user->custom_files[1] = reader.deserialize<uint32_t, LittleEndian>();
    user->custom_files[2] = reader.deserialize<uint32_t, LittleEndian>();
    user->custom_files[3] = reader.deserialize<uint32_t, LittleEndian>();
}

template<typename Observer>
void Client<Observer>::update_user(size_t index, const std::string& data)
{
    if (data.empty())
    {
        return;
    }

    if (index >= this->_users.size() || this->_users.at(index) == nullptr)
    {
        User* user = new User(index);
        BEFORE(Observer, UserCreationObserver);
        this->_update_user(user, data);
        this->_users.emplace(index, user);
        AFTER(UserCreationObserver, user);
    }
    else
    {
        User* user = this->_users.at(index);
        BEFORE(Observer, UserUpdateObserver, user);
        this->_update_user(user, data);
        AFTER(UserUpdateObserver, user);
    }
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
    CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
    VERIFY(limit > 0);

    csgo::message::net::CSVCMsg_GameEventList data;
    data.ParseFromCodedStream(&stream);
    for (csgo::message::net::CSVCMsg_GameEventList_descriptor_t& descriptor : *data.mutable_descriptors())
    {
        BEFORE(Observer, GameEventTypeCreationObserver);
        GameEventType* game_event_type = GameEventType::build(std::move(descriptor));
        this->_game_event_types.emplace(game_event_type);
        AFTER(GameEventTypeCreationObserver, std::as_const(game_event_type));
    }

    VERIFY(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
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
