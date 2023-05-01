#include "csgopy/csgopp/client/string_table.h"

nanobind::class_<StringTableAdapter> StringTableAdapter::bind(nanobind::module_& module_)
{
    return nanobind::class_<StringTableAdapter>(module_, "StringTable");
}
