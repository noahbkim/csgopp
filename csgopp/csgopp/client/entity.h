#pragma once

#include <memory>
#include <vector>
#include <string_view>
#include <absl/container/flat_hash_map.h>

#include "../common/vector.h"
#include "../common/database.h"
#include "data_table.h"
#include "server_class.h"

namespace csgopp::client::entity
{

using csgopp::common::vector::Vector3;
using csgopp::common::database::Delete;
using csgopp::common::database::Database;
using csgopp::common::database::NameTableMixin;
using csgopp::client::data_table::DataTable;
using csgopp::client::server_class::ServerClass;

struct Entity
{
    using Id = uint32_t;
    using Member = DataTable::Value;

    // We can't use DatabaseWithName because we emplace the member property name
    struct MemberDatabase : public Database<Member>, public NameTableMixin<Member>
    {
        using Database<Member>::Database;

        [[nodiscard]] const Member* at(Id member_id) const
        {
            return this->container.at(static_cast<size_t>(member_id));
        }

        void emplace(Member* member) override
        {
            Database::emplace(member);
//            this->by_name.emplace(member->origin()->name, member);
        }
    };

    Id id;
    Vector3 position{};
    ServerClass* server_class;
    MemberDatabase members;

    explicit Entity(Id id, ServerClass* server_class)
        : id(id)
        , server_class(server_class) {}
};

using EntityDatabase = Database<Entity, Delete<Entity>>;

}
