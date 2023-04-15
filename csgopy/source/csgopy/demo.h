#pragma once

#include <nanobind/nanobind.h>
#include <csgopp/demo.h>

namespace header
{

using csgopp::demo::Header;

void bind(nanobind::module_& module_)
{
    nanobind::class_<Header>(module_, "Header")
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
        .def_prop_ro("sign_on_size", [](const Header* self) { return self->sign_on_size; });
}

}
