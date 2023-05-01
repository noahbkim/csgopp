#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "nanobind/nanobind.h"
#include "nanobind/stl/string.h"
#include "nanobind/stl/vector.h"
#include "csgopp/client/user.h"
#include "csgopy/adapter.h"

using csgopp::client::user::User;

struct UserAdapter : public Adapter<const User>
{
    using Adapter::Adapter;

    [[nodiscard]] std::string repr() const
    {
        return "User("
            "id=" + std::to_string(self->id) + ", "
            "index=" + std::to_string(self->index) + ", "
            "name=\"" + self->name + "\""
        ")";
    }

    [[nodiscard]] User::Index index() const { return self->index; }
    [[nodiscard]] uint64_t version() const { return self->version; }
    [[nodiscard]] uint64_t xuid() const { return self->xuid; }
    [[nodiscard]] std::string name() const { return self->name; }
    [[nodiscard]] User::Id id() const { return self->id; }
    [[nodiscard]] std::string guid() const { return self->guid; }
    [[nodiscard]] uint32_t friends_id() const { return self->friends_id; }
    [[nodiscard]] std::string friends_name() const { return self->friends_name; }
    [[nodiscard]] bool is_fake() const { return self->is_fake; }
    [[nodiscard]] bool is_hltv() const { return self->is_hltv; }
    [[nodiscard]] std::vector<uint32_t> custom_files() const
    {
        return {self->custom_files, self->custom_files + 4};
    }
    [[nodiscard]] bool files_downloaded() const { return self->files_downloaded; }

    static nanobind::class_<UserAdapter> bind(nanobind::module_& module_);
};
