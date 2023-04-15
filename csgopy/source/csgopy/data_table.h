#pragma once

#include <nanobind/nanobind.h>
#include <csgopp/client.h>
#include "adapter.h"

using csgopp::client::data_table::DataTable;

struct DataTableAdapter : public Adapter<const DataTable>
{
    using Adapter::Adapter;

    static nanobind::class_<DataTableAdapter> bind(nanobind::module_& module_);
};
