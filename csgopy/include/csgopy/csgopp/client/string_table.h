#pragma once

#include "nanobind/nanobind.h"
#include "csgopp/client/string_table.h"
#include "csgopy/adapter.h"

using csgopp::client::string_table::StringTable;

struct StringTableAdapter : public Adapter<const StringTable>
{
    using Adapter::Adapter;

    static nanobind::class_<StringTableAdapter> bind(nanobind::module_& module);
};
