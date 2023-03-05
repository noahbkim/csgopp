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
using csgopp::error::GameError;
using csgopp::client::data_table::DataTable;
using csgopp::client::data_table::is_array_index;
using csgopp::client::server_class::ServerClass;
using csgopp::client::string_table::StringTable;
using csgopp::client::entity::EntityType;
using csgopp::client::entity::Entity;
using csgopp::client::entity::EntityDatum;
using csgopp::client::game_event::GameEventType;
using csgopp::client::game_event::GameEvent;
using csgopp::client::user::User;

constexpr size_t MAX_EDICT_BITS = 11;
constexpr size_t ENTITY_HANDLE_INDEX_MASK = (1 << MAX_EDICT_BITS) - 1;
constexpr size_t ENTITY_HANDLE_SERIAL_NUMBER_BITS = 10;
constexpr size_t ENTITY_HANDLE_BITS = MAX_EDICT_BITS + ENTITY_HANDLE_SERIAL_NUMBER_BITS;
constexpr size_t INVALID_ENTITY_HANDLE = (1 << ENTITY_HANDLE_BITS) - 1;


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
class Client
{
public:
    using DataTableDatabase = csgopp::client::data_table::DataTableDatabase;
    using ServerClassDatabase = csgopp::client::server_class::ServerClassDatabase;
    using StringTableDatabase = csgopp::client::string_table::StringTableDatabase;
    using EntityDatabase = csgopp::client::entity::EntityDatabase;
    using GameEventTypeDatabase = csgopp::client::game_event::GameEventTypeDatabase;
    using UserDatabase = csgopp::client::user::UserDatabase;

    explicit Client(CodedInputStream& stream);

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
    virtual void advance_packet_unknown(CodedInputStream& stream, uint32_t command);
    virtual void advance_console_command(CodedInputStream& stream);
    virtual void advance_user_command(CodedInputStream& stream);
    virtual void advance_data_tables(CodedInputStream& stream);
    virtual void advance_string_tables(CodedInputStream& stream);
    virtual void advance_custom_data(CodedInputStream& stream);
    virtual bool advance_unknown(CodedInputStream& stream, char command);

    /// Handlers
    virtual inline void before_frame() {}
    virtual inline void on_frame(csgopp::demo::Command::Type command) {}
    virtual inline void before_packet(uint32_t type) {}
    virtual inline void on_packet(uint32_t type) {}
    virtual inline void before_data_table_creation() {}
    virtual inline void on_data_table_creation(const std::shared_ptr<const DataTable>& data_table) {}
    virtual inline void before_server_class_creation() {}
    virtual inline void on_server_class_creation(const std::shared_ptr<const ServerClass>& server_class) {}
    virtual inline void before_string_table_creation() {}
    virtual inline void on_string_table_creation(const std::shared_ptr<const StringTable>& string_table) {}
    virtual inline void before_string_table_update(const std::shared_ptr<const StringTable>& string_table) {}
    virtual inline void on_string_table_update(const std::shared_ptr<const StringTable>& string_table) {}
    virtual inline void before_entity_creation(Entity::Id id, const std::shared_ptr<const ServerClass>& server_class) {}
    virtual inline void on_entity_creation(const std::shared_ptr<const Entity>& entity) {}
    virtual inline void before_entity_update(const std::shared_ptr<const Entity>& entity, const std::vector<uint16_t>& indices) {}
    virtual inline void on_entity_update(const std::shared_ptr<const Entity>& entity, const std::vector<uint16_t>& indices) {}
    virtual inline void before_entity_deletion(const std::shared_ptr<const Entity>& entity) {}
    virtual inline void on_entity_deletion(std::shared_ptr<const Entity>&& entity) {}
    virtual inline void before_game_event_type_creation() {}
    virtual inline void on_game_event_type_creation(const std::shared_ptr<const GameEventType>& game_event_type) {}
    virtual inline void before_game_event() {}
    virtual inline void on_game_event(GameEvent&& event) {}
    virtual inline void before_user_creation(User::Index index) {}
    virtual inline void on_user_creation(const std::shared_ptr<const User>& user) {}
    virtual inline void before_user_update(const std::shared_ptr<const User>& user) {}
    virtual inline void on_user_update(const std::shared_ptr<const User>& user) {}

    [[nodiscard]] const demo::Header& header() const { return this->_header; }
    [[nodiscard]] uint32_t cursor() const { return this->_cursor; }
    [[nodiscard]] uint32_t tick() const { return this->_tick; }
    [[nodiscard]] const DataTableDatabase& data_tables() const { return this->_data_tables; }
    [[nodiscard]] const ServerClassDatabase& server_classes() const { return this->_server_classes; }
    [[nodiscard]] const StringTableDatabase& string_tables() const { return this->_string_tables; }
    [[nodiscard]] const EntityDatabase& entities() const { return this->_entities; }
    [[nodiscard]] const GameEventTypeDatabase& game_event_types() const { return this->_game_event_types; }
    [[nodiscard]] const UserDatabase& users() const { return this->_users; }

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

    void populate_string_table(StringTable& string_table, const std::string& blob, int32_t count);

    void _get_update_indices(BitStream& stream);
    void _update_entity(Entity& entity, BitStream& stream);
    void create_entity(Entity::Id id, BitStream& stream);
    void update_entity(Entity::Id id, BitStream& stream);
    void delete_entity(Entity::Id id);

    void _update_user(User& user, const std::string& data);
    void update_user(size_t index, const std::string& data);
};

}
