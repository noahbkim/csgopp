#pragma once

#include <iostream>
#include <map>

#include <csgopp/network/data_table.h>
#include <csgopp/network/server_class.h>
#include <csgopp/demo.h>
#include <csgopp/game.h>

using csgopp::network::ServerClass;
using csgopp::network::DataTable;
using csgopp::game::ObserverBase;
using csgopp::demo::Command;

struct StructureObserver : public ObserverBase<StructureObserver>
{
    size_t frame_count = 0;
    std::map<Command::Type, size_t> commands;
    size_t packet_count = 0;
    std::map<int32_t, size_t> net_messages;

    struct Frame final : public ObserverBase::Frame
    {
        using ObserverBase::Frame::Frame;

        void handle(Simulation& simulation, Command::Type command) override
        {
            simulation.observer.frame_count += 1;
            simulation.observer.commands[command] += 1;
        }
    };

    struct Packet final : public ObserverBase::Packet
    {
        using ObserverBase::Packet::Packet;

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
            std::cout << "- " << csgopp::demo::describe_command(command) << ": " << count << std::endl;
        }

        std::cout << std::endl << "net messages (" << this->packet_count << "):" << std::endl;
        for (const auto& [net_message, count] : this->net_messages)
        {
            std::cout << "- " << csgopp::demo::describe_net_message(net_message) << ": " << count << std::endl;
        }
    }
};

struct DataObserver : public ObserverBase<DataObserver>
{
    struct SendTableCreate final : public ObserverBase::SendTableCreate
    {
        using ObserverBase::SendTableCreate::SendTableCreate;

        void handle(Simulation& simulation, const DataTable* send_table) override
        {
            std::cout << "send table: " << send_table->name << std::endl;
            for (const auto& [name, property] : send_table->properties)
            {
                std::cout << "  - " << name << " " << csgopp::network::describe(property->type) << std::endl;
            }
        }
    };

    struct ServerClassCreate final : public ObserverBase::ServerClassCreate
    {
        using ObserverBase::ServerClassCreate::ServerClassCreate;

        void handle(Simulation& simulation, const ServerClass* server_class) override
        {
            std::cout << "server class: " << server_class->name << ": " << server_class->data_table->name << std::endl;
        }
    };

    void report() const {}
};
