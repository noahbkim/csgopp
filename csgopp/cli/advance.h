#pragma once

#include <iostream>
#include <fstream>

#include <google/protobuf/io/coded_stream.h>
#include <argparse/argparse.hpp>

#include <csgopp/client.h>

#include "common.h"

using argparse::ArgumentParser;
using csgopp::client::ClientObserverBase;
using csgopp::client::Client;

struct AdvanceObserver : ClientObserverBase<AdvanceObserver>
{
    using ClientObserverBase::ClientObserverBase;

    void on_game_event(Client& client, csgo::message::net::CSVCMsg_GameEvent&& event) override
    {
        if (event.event_name() == "round_end")
        {
            std::cout << "Round end:" << std::endl;

        }
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
        std::ifstream file_stream(path, std::ios::binary);
        IstreamInputStream file_input_stream(&file_stream);
        CodedInputStream coded_input_stream(&file_input_stream);

        try
        {
            Client<AdvanceObserver> client(coded_input_stream);
            while (client.advance(coded_input_stream));
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
