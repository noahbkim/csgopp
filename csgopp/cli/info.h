#pragma once

#include <iostream>
#include <map>

#include <csgopp/network/data_table.h>
#include <csgopp/network/server_class.h>
#include <csgopp/demo.h>
#include <csgopp/game.h>
#include <csgopp/model/user.h>

using csgopp::network::ServerClass;
using csgopp::network::DataTable;
using csgopp::network::StringTable;
using csgopp::model::user::User;
using csgopp::game::SimulationObserverBase;
using csgopp::demo::Command;

struct StructureObserver : public SimulationObserverBase<StructureObserver>
{
    using SimulationObserverBase::SimulationObserverBase;

    size_t frame_count = 0;
    std::map<Command::Type, size_t> commands;
    size_t packet_count = 0;
    std::map<int32_t, size_t> net_messages;

    struct FrameObserver final : public SimulationObserverBase::FrameObserver
    {
        using SimulationObserverBase::FrameObserver::FrameObserver;

        void handle(Simulation& simulation, Command::Type command) override
        {
            simulation.observer.frame_count += 1;
            simulation.observer.commands[command] += 1;
        }
    };

    struct PacketObserver final : public SimulationObserverBase::PacketObserver
    {
        using SimulationObserverBase::PacketObserver::PacketObserver;

        void handle(Simulation& simulation, int32_t net_message) override
        {
            simulation.observer.packet_count += 1;
            simulation.observer.net_messages[net_message] += 1;
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

struct DataObserver : public SimulationObserverBase<DataObserver>
{
    using SimulationObserverBase::SimulationObserverBase;

    struct DataTableCreationObserver final : public SimulationObserverBase::DataTableCreationObserver
    {
        using SimulationObserverBase::DataTableCreationObserver::DataTableCreationObserver;

        void handle(Simulation& simulation, const DataTable* send_table) override
        {
            std::cout << "send table: " << send_table->name << std::endl;
            for (const auto& [name, property] : send_table->properties)
            {
                std::cout << "  - " << name << " " << csgopp::network::describe(property->type) << std::endl;
            }
        }
    };

    struct ServerClassCreationObserver final : public SimulationObserverBase::ServerClassCreationObserver
    {
        using SimulationObserverBase::ServerClassCreationObserver::ServerClassCreationObserver;

        void handle(Simulation& simulation, const ServerClass* server_class) override
        {
            std::cout << "server class: " << server_class->name << ": " << server_class->data_table->name << std::endl;
        }
    };

    void report() const {}
};

struct PlayersObserver : public SimulationObserverBase<PlayersObserver>
{
    std::vector<std::pair<size_t, User>> users;

    using SimulationObserverBase::SimulationObserverBase;

    void on_string_table_creation(Simulation& simulation, StringTable&& string_table) override
    {
        if (string_table.name == "userinfo")
        {
            for (StringTable::Entry& entry : string_table.entries)
            {
                size_t player_index;
                if (entry.index.has_value())
                {
                    player_index = entry.index.value();
                }
                else
                {
                    player_index = std::stoull(entry.string);
                }

                if (!entry.data.empty())
                {
                    auto& [index, user] = simulation.observer.users.emplace_back();
                    index = player_index;
                    user.deserialize(entry.data);
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
