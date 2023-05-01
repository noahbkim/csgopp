#pragma once

#include <nanobind/nanobind.h>
#include <csgopp/common/vector.h>

using csgopp::common::vector::Vector2;
using csgopp::common::vector::Vector3;

struct Vector2Binding
{
    static nanobind::class_<Vector2> bind(nanobind::module_& module_);
};

struct Vector3Binding
{
    static nanobind::class_<Vector3> bind(nanobind::module_& module_);
};
