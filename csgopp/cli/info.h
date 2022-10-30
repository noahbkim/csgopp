#pragma once

#include <iostream>
#include <map>

#include <csgopp/common/code.h>
#include <csgopp/client/data_table.h>
#include <csgopp/client/server_class.h>
#include <csgopp/demo.h>
#include <csgopp/client.h>
#include <csgopp/game/user.h>

using csgopp::client::ServerClass;
using csgopp::client::DataTable;
using csgopp::client::StringTable;
using csgopp::game::user::User;
using csgopp::client::ClientObserverBase;
using csgopp::demo::Command;
using csgopp::common::code::Definition;
using csgopp::common::code::Generator;

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
    size_t server_class_count = 0;
    size_t data_table_count = 0;
    size_t entity_type_count = 0;
    Generator generator;
    std::ofstream out;
    std::ofstream out2;

    explicit DataObserver(Client& client)
        : ClientObserverBase<DataObserver>(client)
        , out("local/server_classes.h")
        , out2("local/prioritized.txt")
    {
        out << "#include <cstdint>" << std::endl;
        out << "#include <string>" << std::endl << std::endl;
        out << "struct Vector3 { float x; float y; float z; };" << std::endl;
        out << "struct Vector2 { float x; float y; };" << std::endl << std::endl;
    }

    void on_data_table_creation(Client& client, const DataTable* data_table) override
    {
        this->data_table_count += 1;
        if (data_table->entity_type)
        {
            data_table->emit(generator.append(data_table->name));
            this->entity_type_count += 1;
            out2 << data_table->entity_type->name << std::endl;
            for (auto& [offset, name] : data_table->entity_type->prioritized)
            {
                out2 << "  " << name << ": " << offset.offset << std::endl;
            }
        }
    }

    void on_server_class_creation(Client& client, const ServerClass* server_class) override
    {
        this->server_class_count += 1;
    }

    void report()
    {
//        sort(this->structures);
        this->generator.write(out);

        std::cout << "server classes: " << this->server_class_count << std::endl;
        std::cout << "data tables: " << this->data_table_count << std::endl;
        std::cout << "entity types: " << this->entity_type_count << std::endl;
    }
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
