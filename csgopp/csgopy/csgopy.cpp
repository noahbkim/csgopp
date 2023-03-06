#include <memory>
#include <utility>
#include <iostream>
#include <fstream>

#include <nanobind/nanobind.h>
#include <nanobind/trampoline.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/shared_ptr.h>

#include <google/protobuf/io/coded_stream.h>

#include <csgopp/client.h>

using namespace nanobind::literals;

using csgopp::client::Client;
using csgopp::client::entity::EntityConstReference;
using csgopp::client::entity::EntityType;
using csgopp::client::Entity;
using csgopp::client::ServerClass;
using csgopp::client::DataTable;
using csgopp::client::GameEventType;
using csgopp::client::GameEvent;
using csgopp::client::StringTable;
using csgopp::client::User;
using csgopp::demo::Header;
using google::protobuf::io::CodedInputStream;
using google::protobuf::io::IstreamInputStream;
using object::Type;

template<typename T>
struct Adapter
{
    std::shared_ptr<T> self;

    Adapter() = default;
    explicit Adapter(auto&& self) : self(std::move(self)) {}
};

struct DataTableAdapter : public Adapter<const DataTable>
{
    using Adapter::Adapter;

    static void bind(nanobind::module_& module)
    {
        nanobind::class_<DataTableAdapter>(module, "DataTable");
    }
};

struct ServerClassAdapter : public Adapter<const ServerClass>
{
    using Adapter::Adapter;

    [[nodiscard]] const std::string& name() const
    {
        return self->name;
    }

    static void bind(nanobind::module_& module)
    {
        nanobind::class_<ServerClassAdapter>(module, "ServerClass")
            .def_prop_ro("name", &ServerClassAdapter::name);
    }
};

struct StringTableAdapter : public Adapter<const StringTable>
{
    using Adapter::Adapter;

    static void bind(nanobind::module_& module)
    {
        nanobind::class_<StringTableAdapter>(module, "StringTable");
    }
};


struct GameEventTypeAdapter : public Adapter<const GameEventType>
{
    using Adapter::Adapter;

    static void bind(nanobind::module_& module)
    {
        nanobind::class_<GameEventTypeAdapter>(module, "GameEventType");
    }
};

struct GameEventAdapter
{
    static void bind(nanobind::module_& module)
    {
        nanobind::class_<GameEvent>(module, "GameEvent");
    }
};

struct UserAdapter : public Adapter<const User>
{
    using Adapter::Adapter;

    [[nodiscard]] User::Index index() const { return self->index; }
    [[nodiscard]] uint64_t version() const { return self->version; }
    [[nodiscard]] uint64_t xuid() const { return self->xuid; }
    [[nodiscard]] std::string name() const { return self->name; }
    [[nodiscard]] User::Id id() const { return self->id; }
    [[nodiscard]] std::string guid() const { return self->guid; }
    [[nodiscard]] uint32_t friends_id() const { return self->friends_id; }
    [[nodiscard]] std::string friends_name() const { return self->friends_name; }
    [[nodiscard]] bool is_fake() const { return self->is_fake; }
    [[nodiscard]] bool is_hltv() const { return self->is_hltv; }
    [[nodiscard]] std::vector<uint32_t> custom_files() const
    {
        return std::vector(self->custom_files, self->custom_files + 4);
    }
    [[nodiscard]] bool files_downloaded() const { return self->files_downloaded; }

    static void bind(nanobind::module_& module)
    {
        nanobind::class_<UserAdapter>(module, "User")
            .def_prop_ro("index", &UserAdapter::index)
            .def_prop_ro("version", &UserAdapter::version)
            .def_prop_ro("xuid", &UserAdapter::xuid)
            .def_prop_ro("name", &UserAdapter::name)
            .def_prop_ro("id", &UserAdapter::id)
            .def_prop_ro("guid", &UserAdapter::guid)
            .def_prop_ro("friends_id", &UserAdapter::friends_id)
            .def_prop_ro("friends_name", &UserAdapter::friends_name)
            .def_prop_ro("is_fake", &UserAdapter::is_fake)
            .def_prop_ro("is_hltv", &UserAdapter::is_hltv)
            .def_prop_ro("custom_files", &UserAdapter::custom_files)
            .def_prop_ro("files_downloaded", &UserAdapter::files_downloaded)
        ;
    }
};

struct EntityTypeAdapter : public Adapter<const EntityType>
{
    using Adapter::Adapter;

    static void bind(nanobind::module_& module)
    {
        nanobind::class_<EntityTypeAdapter>(module, "EntityType");
    }
};

struct EntityAdapter : public Adapter<const Entity>
{
    using Adapter::Adapter;

    [[nodiscard]] Entity::Id id() const
    {
        return this->self->id;
    }

    static void bind(nanobind::module_& module)
    {
        nanobind::class_<EntityAdapter>(module, "Entity")
            .def_prop_ro("id", &EntityAdapter::id)
//        .def_prop_ro("server_class", [](const Entity* entity) { return entity->server_class; })
            .def_prop_ro("server_class_index", [](const Entity* entity) { return entity->server_class->index; })
            ;
    }
};

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

NB_MODULE(csgopy, module) {
    nanobind::exception<csgopp::client::GameError>(module, "GameError");
    DataTableAdapter::bind(module);
    ServerClassAdapter::bind(module);
    StringTableAdapter::bind(module);
    GameEventTypeAdapter::bind(module);
    GameEventAdapter::bind(module);
    UserAdapter::bind(module);
    EntityTypeAdapter::bind(module);
    EntityAdapter::bind(module);

    nanobind::class_<Header>(module, "Header")
        .def_prop_ro("magic", [](const Header* self) { return self->magic; })
        .def_prop_ro("demo_protocol", [](const Header* self) { return self->demo_protocol; })
        .def_prop_ro("network_protocol", [](const Header* self) { return self->network_protocol; })
        .def_prop_ro("server_name", [](const Header* self) { return self->server_name; })
        .def_prop_ro("client_name", [](const Header* self) { return self->client_name; })
        .def_prop_ro("map_name", [](const Header* self) { return self->map_name; })
        .def_prop_ro("game_directory", [](const Header* self) { return self->game_directory; })
        .def_prop_ro("playback_time", [](const Header* self) { return self->playback_time; })
        .def_prop_ro("tick_count", [](const Header* self) { return self->tick_count; })
        .def_prop_ro("frame_count", [](const Header* self) { return self->frame_count; })
        .def_prop_ro("sign_on_size", [](const Header* self) { return self->sign_on_size; })
    ;

    // We need to do this for ClientAdapter base class rather than add template argument because it gets confused
    auto client = nanobind::class_<Client>(module, "_Client")
        .def_prop_ro("header", &ClientAdapter::header)
        .def("cursor", &ClientAdapter::cursor)
        .def("tick", &ClientAdapter::tick)
    ;

    // We have to do this here because of the trampoline (include Client for base class methods)
    nanobind::class_<ClientAdapter, ClientAdapterTrampoline>(module, "Client", client)
        .def(nanobind::init<const std::string&>())
        .def("advance", &ClientAdapter::advance)
        .def("parse", &ClientAdapter::parse)
        .def("before_frame", &ClientAdapter::before_frame)
        .def("on_frame", &ClientAdapter::on_frame)
        .def("before_packet", &ClientAdapter::before_packet)
        .def("on_packet", &ClientAdapter::on_packet)
        .def("before_data_table_creation", &ClientAdapter::before_data_table_creation)
        .def("on_data_table_creation", &ClientAdapter::on_data_table_creation)
        .def("before_server_class_creation", &ClientAdapter::before_server_class_creation)
        .def("on_server_class_creation", &ClientAdapter::on_server_class_creation)
        .def("before_string_table_creation", &ClientAdapter::before_string_table_creation)
        .def("on_string_table_creation", &ClientAdapter::on_string_table_creation)
        .def("before_string_table_update", &ClientAdapter::before_string_table_update)
        .def("on_string_table_update", &ClientAdapter::on_string_table_update)
        .def("before_entity_creation", &ClientAdapter::before_entity_creation)
        .def("on_entity_creation", &ClientAdapter::on_entity_creation)
        .def("before_entity_update", &ClientAdapter::before_entity_update)
        .def("on_entity_update", &ClientAdapter::on_entity_update)
        .def("before_entity_deletion", &ClientAdapter::before_entity_deletion)
        .def("on_entity_deletion", &ClientAdapter::on_entity_deletion)
        .def("before_game_event_type_creation", &ClientAdapter::before_game_event_type_creation)
        .def("on_game_event_type_creation", &ClientAdapter::on_game_event_type_creation)
        .def("before_game_event", &ClientAdapter::before_game_event)
        .def("on_game_event", &ClientAdapter::on_game_event)
        .def("before_user_creation", &ClientAdapter::before_user_creation)
        .def("on_user_creation", &ClientAdapter::on_user_creation)
        .def("before_user_update", &ClientAdapter::before_user_update)
        .def("on_user_update", &ClientAdapter::on_user_update)
    ;
}
