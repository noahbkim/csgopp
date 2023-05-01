#pragma once

#include <string>
#include "nanobind/nanobind.h"
#include "nanobind/stl/string.h"
#include "csgopp/client/server_class.h"
#include "csgopy/adapter.h"

using csgopp::client::server_class::ServerClass;

struct ServerClassAdapter : public Adapter<const ServerClass>
{
    using Adapter::Adapter;

    [[nodiscard]] std::string repr() const
    {
        return "ServerClass(name=\"" + self->name + "\")";
    }

    [[nodiscard]] ServerClass::Index index() const
    {
        return this->self->index;
    }

    [[nodiscard]] const std::string& name() const
    {
        return this->self->name;
    }

    static nanobind::class_<ServerClassAdapter> bind(nanobind::module_& module_);
};
