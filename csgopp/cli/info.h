#pragma once

#include <iostream>
#include <map>

#include <csgopp/client/data_table.h>
#include <csgopp/client/server_class.h>
#include <csgopp/demo.h>
#include <csgopp/client.h>
#include <csgopp/model/user.h>

using csgopp::client::ServerClass;
using csgopp::client::DataTable;
using csgopp::client::StringTable;
using csgopp::model::user::User;
using csgopp::client::ClientObserverBase;
using csgopp::demo::Command;

struct StructureObserver : public ClientObserverBase<StructureObserver>
{
    using ClientObserverBase::ClientObserverBase;

    size_t frame_count = 0;
    std::map<Command::Type, size_t> commands;
    size_t packet_count = 0;
    std::map<int32_t, size_t> net_messages;

    struct FrameObserver final : public ClientObserverBase::FrameObserver
    {
        using ClientObserverBase::FrameObserver::FrameObserver;

        void handle(Client& client, Command::Type command) override
        {
            client.observer.frame_count += 1;
            client.observer.commands[command] += 1;
        }
    };

    struct PacketObserver final : public ClientObserverBase::PacketObserver
    {
        using ClientObserverBase::PacketObserver::PacketObserver;

        void handle(Client& client, int32_t net_message) override
        {
            client.observer.packet_count += 1;
            client.observer.net_messages[net_message] += 1;
        }
    };

    void report() const
    {
        std::cout << std::endl << "frames (" << this->frame_count << "):" << std::endl;
        for (const auto& [command, count] : this->commands)
        {
            std::cout << "  - " << csgopp::demo::describe_command(command) << ": " << count << std::endl;
        }

        std::cout << std::endl << "net messages (" << this->packet_count << "):" << std::endl;
        for (const auto& [net_message, count] : this->net_messages)
        {
            std::cout << "  - " << csgopp::demo::describe_net_message(net_message) << ": " << count << std::endl;
        }
    }
};

struct DataObserver : public ClientObserverBase<DataObserver>
{
    using ClientObserverBase::ClientObserverBase;

    struct DataTableCreationObserver final : public ClientObserverBase::DataTableCreationObserver
    {
        using ClientObserverBase::DataTableCreationObserver::DataTableCreationObserver;

        void handle(Client& client, const DataTable* send_table) override
        {
//            std::cout << "send table: " << send_table->name << std::endl;
//            for (const DataTable::Property* property : send_table->properties)
//            {
//                std::cout << "  - " << property->name << " "
//                    << csgopp::client::data_table::describe(property->type) << std::endl;
//            }
        }
    };

    struct ServerClassCreationObserver final : public ClientObserverBase::ServerClassCreationObserver
    {
        using ClientObserverBase::ServerClassCreationObserver::ServerClassCreationObserver;

        void handle(Client& client, const ServerClass* server_class) override
        {
            for (const DataTable::Property* property : server_class->properties)
            {
                if (property->type == DataTable::Property::Type::ARRAY)
                {

                }
            }
        }
    };

    void report() const {}
};

struct PlayersObserver : public ClientObserverBase<PlayersObserver>
{
    std::vector<std::pair<size_t, User>> users;

    using ClientObserverBase::ClientObserverBase;

    void on_string_table_creation(Client& client, const StringTable* string_table) override
    {
        if (string_table->name == "userinfo")
        {
            for (size_t i = 0; i < string_table->entries.size(); ++i)
            {
                const StringTable::Entry* entry = string_table->entries.at(i);
                if (!entry->data.empty())
                {
                    auto& [index, user] = client.observer.users.emplace_back();
                    index = i;
                    user.deserialize(entry->data);
                }
            }
        }
    }

    void report() const
    {
        std::cout << std::endl;
        for (const auto& [index, user] : this->users)
        {
            std::cout << user.name << std::endl;
            std::cout << "  index: " << index << std::endl;
            std::cout << "  version: " << user.version << std::endl;
            std::cout << "  xuid: " << user.xuid << std::endl;
            std::cout << "  user_id: " << user.user_id << std::endl;
            std::cout << "  guid: " << user.guid << std::endl;
            std::cout << "  friends_id: " << user.friends_id << std::endl;
            std::cout << "  friends_name: " << user.friends_name << std::endl;
            std::cout << "  is_fake: " << user.is_fake << std::endl;
            std::cout << "  is_hltv: " << user.is_hltv << std::endl;
        }
    }
};
