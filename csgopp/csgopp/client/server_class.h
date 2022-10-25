#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <absl/container/flat_hash_map.h>

#include "../common/database.h"
#include "../error.h"
#include "data_table.h"

namespace csgopp::client::server_class
{

using google::protobuf::io::CodedInputStream;
using csgopp::common::database::Delete;
using csgopp::common::database::Database;
using csgopp::common::database::DatabaseWithNameId;
using csgopp::error::GameError;
using csgopp::client::data_table::DataTable;

struct RenamedPropertyDatabase : public Database<DataTable::Property>
{
    // We own the string here
    using NameTable = absl::flat_hash_map<std::string, Value*>;
    NameTable by_name{};

    RenamedPropertyDatabase() = default;
    explicit RenamedPropertyDatabase(size_t reserved) : Database<Value>(reserved), by_name(reserved) {}

    [[nodiscard]] Value*& at(const std::string& name)
    {
        return this->by_name.at(name);
    }

    [[nodiscard]] const Value* at(const std::string& name) const
    {
        return this->by_name.at(name);
    }

    [[nodiscard]] bool contains(const std::string& name)
    {
        return this->by_name.contains(name);
    }

    void emplace(Value* member) override
    {
        throw GameError("must overwrite name!");
    }

    template<typename S>
    void emplace(S&& name, Value* member)
    {
        Database::emplace(member);
        this->by_name.emplace(name, member);
    }

    void reserve(size_t count) override
    {
        Database::reserve(count);
        this->by_name.reserve(count);
    }
};

struct ServerClass
{
    using Id = uint16_t;

    Id id{};
    std::string name;
    DataTable* data_table{nullptr};
    ServerClass* base_class{nullptr};

    template<typename Callback>
    void visit(Callback callback) const;

    template<typename Callback>
    void visit(Callback callback);

    template<typename Callback>
    static void visit(const DataTable* data_table, Callback callback);

    template<typename Callback>
    static void visit(DataTable* data_table, Callback callback);

private:
    template<typename Callback>
    static void visit(const DataTable* data_table, Callback callback, const std::string& prefix);

    template<typename Callback>
    static void visit(DataTable* data_table, Callback callback, const std::string& prefix);
};

using ServerClassDatabase = DatabaseWithNameId<ServerClass, Delete<ServerClass>>;

template<typename Callback>
void ServerClass::visit(Callback callback)
{
    ServerClass::visit(this->data_table, callback);
}

template<typename Callback>
void ServerClass::visit(Callback callback) const
{
    ServerClass::visit(this->data_table, callback);
}

template<typename Callback>
void ServerClass::visit(DataTable* data_table, Callback callback)
{
    ServerClass::visit(data_table, callback, "");
}

template<typename Callback>
void ServerClass::visit(const DataTable* data_table, Callback callback)
{
    ServerClass::visit(data_table, callback, "");
}

std::string join(const std::string& head, const std::string& tail, bool skip = false);

template<typename Callback>
void ServerClass::visit(DataTable* data_table, Callback callback, const std::string& prefix)
{
    for (DataTable::Property* property : data_table->properties)
    {
        if (property->excluded())
        {
            continue;
        }

        auto* data_table_property = dynamic_cast<DataTable::DataTableProperty*>(property);
        if (data_table_property != nullptr)
        {
            visit(
                data_table_property->data_table,
                callback,
                join(prefix, data_table_property->name, data_table_property->collapsible()));
        }
        else
        {
            callback(prefix, property);
        }
    }
}

template<typename Callback>
void ServerClass::visit(const DataTable* data_table, Callback callback, const std::string& prefix)
{
    for (const DataTable::Property* property : data_table->properties)
    {
        if (property->excluded())
        {
            continue;
        }

        const auto* data_table_property = dynamic_cast<const DataTable::DataTableProperty*>(property);
        if (data_table_property != nullptr)
        {
            visit(
                data_table_property->data_table,
                callback,
                join(prefix, data_table_property->name, data_table_property->collapsible()));
        }
        else
        {
            callback(prefix, property);
        }
    }
}

}
