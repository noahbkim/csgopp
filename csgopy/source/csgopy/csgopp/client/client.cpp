#include "csgopy/csgopp/client.h"

nanobind::class_<ClientAdapter, ClientAdapterTrampoline> ClientAdapter::bind(nanobind::module_& module_)
{
    // We need to do this for ClientAdapter base class rather than add template argument because it gets confused
    auto client = nanobind::class_<Client>(module_, "BaseClient")
        .def("header", &ClientAdapter::header)
        .def("cursor", &ClientAdapter::cursor)
        .def("tick", &ClientAdapter::tick)
        ;

    // We have to do this here because of the trampoline (include Client for base class methods)
    return nanobind::class_<ClientAdapter, ClientAdapterTrampoline>(module_, "Client", client)
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
