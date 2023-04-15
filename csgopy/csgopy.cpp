#include <memory>
#include <utility>
#include <iostream>
#include <fstream>
#include <typeindex>
#include <absl/container/flat_hash_map.h>

#include <nanobind/nanobind.h>
#include <nanobind/trampoline.h>
#include <nanobind/stl/shared_ptr.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include <google/protobuf/io/coded_stream.h>

#include <csgopp/client.h>

using namespace nanobind::literals;

using csgopp::client::Client;
using csgopp::client::DataTable;
using csgopp::client::entity::EntityLens;
using csgopp::client::entity::EntityDatum;
using csgopp::client::entity::EntityConstantReference;
using csgopp::client::entity::EntityType;
using csgopp::client::Entity;
using csgopp::client::GameEvent;
using csgopp::client::GameEventType;
using csgopp::client::ServerClass;
using csgopp::client::StringTable;
using csgopp::client::User;
using csgopp::common::vector::Vector2;
using csgopp::common::vector::Vector3;
using csgopp::demo::Header;
using google::protobuf::io::CodedInputStream;
using google::protobuf::io::IstreamInputStream;
using object::ConstantReference;
using object::View;
using object::Lens;
using object::IndexError;
using object::Instance;
using object::MemberError;
using object::Type;
using object::TypeError;
using object::ValueType;

template<typename T>
struct Adapter
{
    std::shared_ptr<T> self;

    Adapter() = default;
    explicit Adapter(std::shared_ptr<T> self) : self(std::move(self)) {}
};

struct DataTableAdapter : public Adapter<const DataTable>
{
    using Adapter::Adapter;

    static nanobind::class_<DataTableAdapter> bind(nanobind::module_& module)
    {
        return nanobind::class_<DataTableAdapter>(module, "DataTable");
    }
};

struct ServerClassAdapter : public Adapter<const ServerClass>
{
    using Adapter::Adapter;

    [[nodiscard]] ServerClass::Index index() const
    {
        return this->self->index;
    }

    [[nodiscard]] const std::string& name() const
    {
        return this->self->name;
    }

    static nanobind::class_<ServerClassAdapter> bind(nanobind::module_& module);
};

struct StringTableAdapter : public Adapter<const StringTable>
{
    using Adapter::Adapter;

    static nanobind::class_<StringTableAdapter> bind(nanobind::module_& module)
    {
        return nanobind::class_<StringTableAdapter>(module, "StringTable");
    }
};


struct GameEventTypeAdapter : public Adapter<const GameEventType>
{
    using Adapter::Adapter;

    static nanobind::class_<GameEventTypeAdapter> bind(nanobind::module_& module)
    {
        return nanobind::class_<GameEventTypeAdapter>(module, "GameEventType");
    }
};

struct GameEventAdapter
{
    static nanobind::class_<GameEvent> bind(nanobind::module_& module)
    {
        return nanobind::class_<GameEvent>(module, "GameEvent");
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

    static nanobind::class_<UserAdapter> bind(nanobind::module_& module)
    {
        return nanobind::class_<UserAdapter>(module, "User")
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

struct ViewAdapter
{
    static nanobind::class_<View> bind(nanobind::module_& module, nanobind::class_<View>& base);
};

struct EntityAccessorAdapter
{
    static nanobind::class_<EntityLens> bind(nanobind::module_& module, nanobind::class_<Lens>& base)
    {
        return nanobind::class_<EntityLens>(module, "EntityAccessor", base)
        ;
    }
};

struct EntityTypeAdapter : public Adapter<const EntityType>
{
    using Adapter::Adapter;

    [[nodiscard]] EntityLens property(size_t index) const
    {
        const EntityDatum& datum = this->self->prioritized.at(index);
        return EntityLens(
            this->self,
            datum.type,
            datum.offset,
            datum.property,
            datum.parent
        );
    }

    [[nodiscard]] Lens at_name(const std::string& name) const
    {
        return Lens(this->self)[name];
    }

    [[nodiscard]] Lens at_index(size_t index) const
    {
        return Lens(this->self)[index];
    }

    static nanobind::class_<EntityTypeAdapter> bind(nanobind::module_& module)
    {
        return nanobind::class_<EntityTypeAdapter>(module, "EntityType")
            .def("property", &EntityTypeAdapter::property)
            .def("__getitem__", &EntityTypeAdapter::at_name)
            .def("__getitem__", &EntityTypeAdapter::at_index)
        ;
    }
};

nanobind::class_<ServerClassAdapter> ServerClassAdapter::bind(nanobind::module_& module)
{
    return nanobind::class_<ServerClassAdapter>(module, "ServerClass")
        .def("index", &ServerClassAdapter::index)
        .def("name", &ServerClassAdapter::name)
        .def("type", [](const ServerClassAdapter* adapter) { return EntityTypeAdapter(adapter->self->type()); })
        ;
}

template<typename T = Type>
struct TypeAdapter : public Adapter<const T>
{
    using Adapter<const T>::Adapter;

    static nanobind::class_<TypeAdapter<T>> bind(nanobind::module_& module, const char* name)
    {
        return nanobind::class_<TypeAdapter<T>>(module, name)
            .def("__repr__", [name](const TypeAdapter<T>* adapter)
            {
                return std::string(name) + "<" + adapter->self->represent() + ">";
            })
        ;
    }
};

nanobind::class_<Accessor> AccessorAdapter::bind(nanobind::module_& module, nanobind::class_<Lens>& base)
{
    return nanobind::class_<Accessor>(module, "Accessor", base)
        .def("origin", [](const Accessor* accessor) { return TypeAdapter<Type>(accessor->origin); })
        .def("__getitem__", [](const Accessor* self, const std::string& name) { return self->operator[](name); })
        .def("__getitem__", [](const Accessor* self, size_t index) { return self->operator[](index); })
    ;
}

struct LensAdapter
{
    static nanobind::class_<Lens> bind(nanobind::module_& module)
    {
        return nanobind::class_<Lens>(module, "Lens")
            .def("type", [](const Lens* self) { return TypeAdapter<Type>(self->type); })
            .def("offset", [](const Lens* self) { return self->offset; })
            .def("is_equal", &Lens::is_equal)
            .def("is_subset_of", &Lens::is_subset_of)
            .def("is_strict_subset_of", &Lens::is_strict_subset_of)
            .def("is_superset_of", &Lens::is_superset_of)
            .def("is_strict_superset_of", &Lens::is_strict_superset_of)
        ;
    }
};

template<typename T>
nanobind::object cast(const char* address)
{
    return nanobind::cast(*reinterpret_cast<const T*>(address));
}

struct ConstantReferenceAdapter
{
    using Caster = nanobind::object (*)(const char*);
    using CasterMap = absl::flat_hash_map<const std::type_info*, Caster>;
    static CasterMap casters;

    static nanobind::class_<ConstantReference> bind(nanobind::module_& module, nanobind::class_<Lens>& base)
    {
        return nanobind::class_<ConstantReference>(module, "ConstantReference", base)
            .def("__getitem__", [](const ConstantReference* self, const std::string& name) { return self->operator[](name); })
            .def("__getitem__", [](const ConstantReference* self, size_t index) { return self->operator[](index); })
            .def("type", [](const ConstantReference* self) { return TypeAdapter<Type>(self->type); })
            .def("value", [](const ConstantReference* self)
            {
                auto* value_type = dynamic_cast<const ValueType*>(self->type.get());
                if (value_type != nullptr)
                {
                    Caster caster = ConstantReferenceAdapter::casters[&value_type->info()];
                    return caster(self->address());
                }
                else
                {
                    throw TypeError("cast is only available for values!");
                }
            })
        ;
    }
};

ConstantReferenceAdapter::CasterMap ConstantReferenceAdapter::casters{
    {&typeid(bool), &cast<bool>},
    {&typeid(uint32_t), &cast<uint32_t>},
    {&typeid(int32_t), &cast<int32_t>},
    {&typeid(float), &cast<float>},
    {&typeid(Vector2), &cast<Vector2>},
    {&typeid(Vector3), &cast<Vector3>},
    {&typeid(std::string), &cast<std::string>},
    {&typeid(uint64_t), &cast<uint64_t>},
    {&typeid(int64_t), &cast<int64_t>},
};

struct EntityConstantReferenceAdapter
{
    static nanobind::class_<EntityConstantReference> bind(nanobind::module_& module, nanobind::class_<ConstantReference>& base)
    {
        return nanobind::class_<EntityConstantReference>(module, "EntityConstantReference", base);
    }
};

template<typename T>
struct InstanceAdapter : public Adapter<const Instance<const T>>
{
    using Adapter<const Instance<const T>>::Adapter;

    [[nodiscard]] TypeAdapter<const T> type() const { return TypeAdapter<const T>(this->self->type); }
    [[nodiscard]] ConstantReference get_name(const std::string& name) const { return this->self->operator[](name); }
    [[nodiscard]] ConstantReference get_index(size_t index) const { return this->self->operator[](index); }

    static nanobind::class_<InstanceAdapter<T>> bind(nanobind::module_& module, const char* name)
    {
        return nanobind::class_<InstanceAdapter<T>>(module, name)
            .def("type", &InstanceAdapter::type)
            .def("__getitem__", &InstanceAdapter::get_name)
            .def("__getitem__", &InstanceAdapter::get_index)
        ;
    }
};

struct EntityAdapter : public Adapter<const Entity>
{
    using Adapter::Adapter;

    [[nodiscard]] Entity::Id id() const
    {
        return this->self->id;
    }

    [[nodiscard]] ServerClassAdapter server_class() const
    {
        return ServerClassAdapter(this->self->server_class);
    }

    [[nodiscard]] ServerClass::Index server_class_index() const
    {
        return this->self->server_class->index;
    }

    [[nodiscard]] EntityConstantReference property(size_t index) const
    {
        return this->self->at(index);
    }

    static nanobind::class_<EntityAdapter> bind(nanobind::module_& module)
    {
        auto base = InstanceAdapter<EntityType>::bind(module, "EntityTypeInstance");
        return nanobind::class_<EntityAdapter>(module, "Entity", base)
            .def("id", &EntityAdapter::id)
            .def("server_class", &EntityAdapter::server_class)
            .def("server_class_index", &EntityAdapter::server_class_index)
            .def("property", &EntityAdapter::property)
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

    auto object_error = nanobind::exception<object::Error>(module, "ObjectError");
    nanobind::exception<object::MemberError>(module, "ObjectMemberError", object_error);
    nanobind::exception<object::IndexError>(module, "ObjectIndexError", object_error);
    nanobind::exception<object::TypeError>(module, "ObjectTypeError", object_error);

    DataTableAdapter::bind(module);
    ServerClassAdapter::bind(module);
    StringTableAdapter::bind(module);
    GameEventTypeAdapter::bind(module);
    GameEventAdapter::bind(module);
    UserAdapter::bind(module);
    auto lens_class = LensAdapter::bind(module);
    auto accessor_class = AccessorAdapter::bind(module, lens_class);
    EntityAccessorAdapter::bind(module, accessor_class);
    TypeAdapter<Type>::bind(module, "Type");
    auto const_reference_class = ConstantReferenceAdapter::bind(module, lens_class);
    EntityConstantReferenceAdapter::bind(module, const_reference_class);
    EntityTypeAdapter::bind(module);
    EntityAdapter::bind(module);

    nanobind::class_<Vector2>(module, "Vector2")
        .def_prop_ro("x", [](const Vector2* self) { return self->x; })
        .def_prop_ro("y", [](const Vector2* self) { return self->y; })
    ;

    nanobind::class_<Vector3>(module, "Vector3")
        .def_prop_ro("x", [](const Vector3* self) { return self->x; })
        .def_prop_ro("y", [](const Vector3* self) { return self->y; })
        .def_prop_ro("z", [](const Vector3* self) { return self->z; })
    ;

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
    auto client = nanobind::class_<Client>(module, "BaseClient")
        .def("header", &ClientAdapter::header)
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
