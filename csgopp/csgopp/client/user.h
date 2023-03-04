#pragma once

#include <cstdint>
#include <string>

#include "../common/database.h"
#include "../client/string_table.h"

namespace csgopp::client::user
{

using csgopp::client::string_table::StringTable;
using csgopp::common::database::DatabaseWithId;

struct User
{
    using Id = int32_t;
    using Index = uint16_t;

    Index index{};

    uint64_t version{};
    uint64_t xuid{};
    std::string name{};
    Id id{};  // Canonically user_id
    std::string guid{};

    uint32_t friends_id{};
    std::string friends_name{};

    bool is_fake{};
    bool is_hltv{};

    uint32_t custom_files[4]{};
    uint8_t files_downloaded{};

    explicit User(Index index) : index(index)
    {
    }
};

struct UserDatabase : public DatabaseWithId<User>
{
    using IndexTable = absl::flat_hash_map<typename User::Index, std::shared_ptr<User>>;
    IndexTable by_index;

    [[nodiscard]] std::shared_ptr<User> at_index(User::Index index)
    {
        return this->by_index.at(index);
    }

    [[nodiscard]] std::shared_ptr<const User> at_index(User::Index index) const
    {
        return this->by_index.at(index);
    }

    [[nodiscard]] bool has_index(User::Id index)
    {
        return this->by_index.contains(index);
    }

    void emplace(std::shared_ptr<User>&& user) override
    {
        this->by_index.emplace(user->index, user);
        DatabaseWithId<User>::emplace(std::move(user));
    }

    void emplace(size_t index, std::shared_ptr<User>&& user) override
    {
        this->by_index.emplace(user->index, user);
        DatabaseWithId<User>::emplace(index, std::move(user));
    }

    void reserve(size_t count) override
    {
        DatabaseWithId<User>::reserve(count);
        this->by_index.reserve(count);
    }
};

}
