#pragma once

#include <iostream>
#include <fstream>

#include <google/protobuf/io/coded_stream.h>
#include <argparse/argparse.hpp>

#include <csgopp/client.h>

#include "common.h"

using argparse::ArgumentParser;
using csgopp::client::Client;
using csgopp::client::GameEvent;
using csgopp::client::User;
using csgopp::client::Entity;

struct AdvanceClient final : Client
{
    // Pack this in here since it's opaque in Python
    std::ifstream file_stream;
    IstreamInputStream file_input_stream;
    CodedInputStream coded_input_stream;

    explicit AdvanceClient(auto path)
        : file_input_stream(&this->file_stream)
        , coded_input_stream(&this->file_input_stream)
    {
        this->file_stream.open(path, std::ios::binary);
        this->_header = csgopp::demo::Header(this->coded_input_stream);
    }

    bool advance()
    {
        return Client::advance(this->coded_input_stream);
    }
};

struct AdvanceCommand
{
    std::string name;
    ArgumentParser parser;

    explicit AdvanceCommand(ArgumentParser& root) : name("advance"), parser(name)
    {
        this->parser.add_description("advance through the demo to benchmark and check for errors");
        this->parser.add_argument("demo");
        root.add_subparser(this->parser);
    }

    [[nodiscard]] int main() const
    {
        Timer timer;
        std::string path = this->parser.get("demo");
        if (!std::filesystem::exists(path))
        {
            std::cerr << "No such file " << std::filesystem::absolute(path) << std::endl;
            return -1;
        }

        try
        {
            Timer client_timer;

            AdvanceClient client(path);
            while (client.advance());

            uint32_t frames = client.cursor();
            uint32_t ms = client_timer.elapsed();
            float rate = static_cast<float>(frames) / static_cast<float>(ms);

            std::cout << "magic: " << client.header().magic << std::endl;
            std::cout << "demo_protocol: " << client.header().demo_protocol << std::endl;
            std::cout << "network_protocol: " << client.header().network_protocol << std::endl;
            std::cout << "server_name: " << client.header().server_name << std::endl;
            std::cout << "client_name: " << client.header().client_name << std::endl;
            std::cout << "map_name: " << client.header().map_name << std::endl;
            std::cout << "game_directory: " << client.header().game_directory << std::endl;
            std::cout << "playback_time: " << client.header().playback_time << std::endl;
            std::cout << "tick_count: " << client.header().tick_count << std::endl;
            std::cout << "frame_count: " << client.header().frame_count << std::endl;
            std::cout << "sign_on_size: " << client.header().sign_on_size << std::endl;
            std::cout << "advanced " << frames << " frames in " << ms << " ms (" << rate << " f/ms)" << std::endl;
        }
        catch (const csgopp::error::GameError& error)
        {
            std::cerr << error.message() << std::endl;
            return -1;
        }

        std::cout << "finished in " << timer << std::endl;
        return 0;
    }
};
