#include <memory>
#include <utility>
#include <iostream>
#include <fstream>

#include <nanobind/nanobind.h>
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
using csgopp::client::User;
using google::protobuf::io::CodedInputStream;
using google::protobuf::io::IstreamInputStream;
using object::Type;

template<typename T>
struct Adapter
{
    std::shared_ptr<T> self;

    Adapter() = default;
    explicit Adapter(std::shared_ptr<T> self) : self(std::move(self)) {}
};

struct ServerClassAdapter : public Adapter<const ServerClass>
{

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

class ClientAdapter final : public Client
{
public:
    using Client::Client;
};

void print(const std::string& string)
{
    std::cout << string << std::endl;
}

void parse(const std::string& path, nanobind::object observer)
{
    std::ifstream file_stream(path, std::ios::binary);
    IstreamInputStream file_input_stream(&file_stream);
    CodedInputStream coded_input_stream(&file_input_stream);

    Client client(coded_input_stream, std::move(observer));
    while (client.advance(coded_input_stream));
}

NB_MODULE(csgopy, module) {
    nanobind::exception<csgopp::client::GameError>(module, "GameError");
    module.def("parse", &parse);
    UserAdapter::bind(module);
    EntityAdapter::bind(module);
}
