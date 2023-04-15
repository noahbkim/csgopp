#pragma once

#include <fstream>

#include <google/protobuf/io/coded_stream.h>
#include <nanobind/nanobind.h>
#include <nanobind/trampoline.h>

#include <csgopp/client.h>

#include "data_table.h"
#include "entity.h"
#include "game_event.h"
#include "server_class.h"
#include "string_table.h"
#include "user.h"

using csgopp::client::Client;
using csgopp::client::Entity;
using csgopp::client::GameEvent;
using csgopp::client::GameEventType;
using csgopp::client::StringTable;
using csgopp::client::User;
using google::protobuf::io::CodedInputStream;
using google::protobuf::io::IstreamInputStream;

// Only look for candidate in Python, do nothing if not found; lookup false means not actually pure
#define NB_OVERRIDE_PURE_ONLY_VOID(name, ...)                                  \
    nanobind::handle nb_key = nb_trampoline.lookup(#name, false);              \
    if (nb_key.is_valid()) {                                                   \
        nanobind::gil_scoped_acquire nb_guard;                                 \
        nb_trampoline.base().attr(nb_key)(__VA_ARGS__);                        \
    }

struct ClientAdapter : public Client
{
    // Pack this in here since it's opaque in Python
    std::ifstream file_stream;
    IstreamInputStream file_input_stream;
    CodedInputStream coded_input_stream;

    explicit ClientAdapter(auto path)
        : file_input_stream(&this->file_stream)
        , coded_input_stream(&this->file_input_stream)
    {
        this->file_stream.open(path, std::ios::binary);
        this->_header = csgopp::demo::Header(this->coded_input_stream);
    }

    /// Inheritance compatibility bindings
    [[maybe_unused]] void on_data_table_creation(const DataTableAdapter&) {}
    [[maybe_unused]] void on_server_class_creation(const ServerClassAdapter&) {}
    [[maybe_unused]] void on_string_table_creation(const StringTableAdapter&) {}
    [[maybe_unused]] void before_string_table_update(const StringTableAdapter&) {}
    [[maybe_unused]] void on_string_table_update(const StringTableAdapter&) {}
    [[maybe_unused]] void before_entity_creation(Entity::Id, const ServerClassAdapter&) {}
    [[maybe_unused]] void on_entity_creation(const UserAdapter&) {}
    [[maybe_unused]] void before_entity_update(const EntityAdapter&, const std::vector<uint16_t>&) {}
    [[maybe_unused]] void on_entity_update(const EntityAdapter&, const std::vector<uint16_t>&) {}
    [[maybe_unused]] void before_entity_deletion(const EntityAdapter&) {}
    [[maybe_unused]] void on_entity_deletion(const EntityAdapter&) {}
    [[maybe_unused]] void before_user_update(const UserAdapter&) {}
    [[maybe_unused]] void on_user_creation(const UserAdapter&) {}
    [[maybe_unused]] void on_user_update(const UserAdapter&) {}
    [[maybe_unused]] void on_game_event_type_creation(const GameEventTypeAdapter&) {}

    bool advance()
    {
        return Client::advance(this->coded_input_stream);
    }

    void parse()
    {
        while (Client::advance(coded_input_stream));
    }

    static nanobind::class_<ClientAdapter, struct ClientAdapterTrampoline> bind(nanobind::module_& module_);
};

struct ClientAdapterTrampoline final : public ClientAdapter
{
    NB_TRAMPOLINE(ClientAdapter, 26);

    void before_frame() override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(before_frame);
    }

    void on_frame(csgopp::demo::Command::Type command) override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(on_frame, command);
    }

    void before_packet(uint32_t type) override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(before_packet, type);
    }

    void on_packet(uint32_t type) override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(on_packet, type);
    }

    void before_data_table_creation() override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(before_data_table_creation);
    }

    void on_data_table_creation(const std::shared_ptr<const DataTable>& data_table) override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(on_data_table_creation, DataTableAdapter(data_table));
    }

    void before_server_class_creation() override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(before_server_class_creation);
    }

    void on_server_class_creation(const std::shared_ptr<const ServerClass>& server_class) override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(on_server_class_creation, ServerClassAdapter(server_class));
    }

    void before_string_table_creation() override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(before_string_table_creation);
    }

    void on_string_table_creation(const std::shared_ptr<const StringTable>& string_table) override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(on_string_table_creation, StringTableAdapter(string_table));
    }

    void before_string_table_update(const std::shared_ptr<const StringTable>& string_table) override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(before_string_table_update, StringTableAdapter(string_table));
    }

    void on_string_table_update(const std::shared_ptr<const StringTable>& string_table) override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(on_string_table_update, StringTableAdapter(string_table));
    }

    void before_entity_creation(Entity::Id id, const std::shared_ptr<const ServerClass>& server_class) override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(before_entity_creation, id, ServerClassAdapter(server_class));
    }

    void on_entity_creation(const std::shared_ptr<const Entity>& entity) override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(on_entity_creation, EntityAdapter(entity));
    }

    void before_entity_update(const std::shared_ptr<const Entity>& entity, const std::vector<uint16_t>& indices) override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(before_entity_update, EntityAdapter(entity), indices);
    }

    void on_entity_update(const std::shared_ptr<const Entity>& entity, const std::vector<uint16_t>& indices) override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(on_entity_update, EntityAdapter(entity), indices);
    }

    void before_entity_deletion(const std::shared_ptr<const Entity>& entity) override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(before_entity_deletion, EntityAdapter(entity));
    }

    void on_entity_deletion(std::shared_ptr<const Entity>&& entity) override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(on_entity_deletion, EntityAdapter(std::move(entity)));
    }

    void before_game_event_type_creation() override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(before_game_event_type_creation);
    }

    void on_game_event_type_creation(const std::shared_ptr<const GameEventType>& game_event_type) override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(on_game_event_type_creation, GameEventTypeAdapter(game_event_type));
    }

    void before_game_event() override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(before_game_event);
    }

    void on_game_event(GameEvent&& event) override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(on_game_event, event);
    }

    void before_user_creation(User::Index index) override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(before_user_creation, index);
    }

    void on_user_creation(const std::shared_ptr<const User>& user) override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(on_user_creation, UserAdapter(user));
    }

    void before_user_update(const std::shared_ptr<const User>& user) override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(before_user_update, UserAdapter(user));
    }

    void on_user_update(const std::shared_ptr<const User>& user) override
    {
        NB_OVERRIDE_PURE_ONLY_VOID(on_user_update, UserAdapter(user));
    }
};
