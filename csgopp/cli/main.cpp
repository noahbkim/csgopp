#include <iostream>
#include <fstream>
#include <map>

#include <csgopp/demo.h>
#include <csgopp/game.h>

using namespace csgopp::demo;
using csgopp::game::Simulation;
using csgopp::common::reader::StreamReader;
using csgopp::game::ObserverBase;

struct Observer : public ObserverBase<Observer>
{
    size_t frame_count = 0;
    std::map<Command, size_t> commands;
    std::map<int32_t, size_t> net_messages;

    struct Frame final : public ObserverBase::Frame
    {
        using ObserverBase::Frame::Frame;

        void handle(Simulation& simulation, Command command) override
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
            simulation.observer.net_messages[net_message] += 1;
        }
    };
};

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "must pass a demo" << std::endl;
        return -1;
    }

    try
    {
        StreamReader<std::ifstream> reader(argv[1], std::ios::binary);
        Simulation<Observer> simulation(reader);

        std::cout << "magic: " << simulation.header().magic << std::endl;
        std::cout << "demo_protocol: " << simulation.header().demo_protocol << std::endl;
        std::cout << "network_protocol: " << simulation.header().network_protocol << std::endl;
        std::cout << "server_name: " << simulation.header().server_name << std::endl;
        std::cout << "client_name: " << simulation.header().client_name << std::endl;
        std::cout << "map_name: " << simulation.header().map_name << std::endl;
        std::cout << "game_directory: " << simulation.header().game_directory << std::endl;
        std::cout << "playback_time: " << simulation.header().playback_time << std::endl;
        std::cout << "tick_count: " << simulation.header().tick_count << std::endl;
        std::cout << "frame_count: " << simulation.header().frame_count << std::endl;
        std::cout << "sign_on_size: " << simulation.header().sign_on_size << std::endl;

        try
        {
            while (simulation.advance(reader));
            std::cout << "finished with frame_count: " << simulation.observer.frame_count << std::endl;

            std::cout << std::endl << "frames:" << std::endl;
            for (const auto& [command, count] : simulation.observer.commands)
            {
                std::cout << "- " << describe(command) << ": " << count << std::endl;
            }

            std::cout << std::endl << "net messages:" << std::endl;
            for (const auto& [net_message, count] : simulation.observer.net_messages)
            {
                std::cout << "- " << describe_net_message(net_message) << ": " << count << std::endl;
            }
        }
        catch (csgopp::error::Error& error)
        {
            std::cerr << "caught exception: " << error.message() << std::endl;
            return -1;
        }
    }
    catch (csgopp::error::Error& error)
    {
        std::cerr << "caught exception: " << error.message() << std::endl;
        return -1;
    }
}
