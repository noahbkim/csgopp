#pragma once

#include <nanobind/nanobind.h>
#include <csgopp/client/server_class.h>
#include "adapter.h"

using csgopp::client::server_class::ServerClass;

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

    static nanobind::class_<ServerClassAdapter> bind(nanobind::module_& module_);
};
