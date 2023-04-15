#include "data_table.h"

nanobind::class_<DataTableAdapter> DataTableAdapter::bind(nanobind::module_& module_)
{
    return nanobind::class_<DataTableAdapter>(module_, "DataTable");
}
