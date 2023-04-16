#pragma once

#include <nanobind/nanobind.h>
#include <csgopp/demo.h>

using csgopp::demo::Header;

struct HeaderBinding
{
    static nanobind::class_<Header> bind(nanobind::module_& module_);
};
