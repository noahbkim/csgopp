#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>

#include <google/protobuf/io/coded_stream.h>
#include <argparse/argparse.hpp>

#include <csgopp/client.h>

#include "common.h"

using argparse::ArgumentParser;
using csgopp::client::ClientObserverBase;
using csgopp::client::Client;
using csgopp::client::User;

struct SummaryObserver : ClientObserverBase<SummaryObserver>
{
    using ClientObserverBase::ClientObserverBase;

    void on_user_creation(Client& client, const User* user) override
    {
        std::cout << "User " << user->name << " connected!" << std::endl;
    }

    struct UserUpdateObserver final : public ClientObserverBase::UserUpdateObserver
    {
        std::string old_name;

        UserUpdateObserver(Client& client, const User* user) : old_name(user->name) {}

        void handle(Client &client, const User* user) override
        {
            if (user->name != old_name)
            {
                std::cout << "User " << old_name << " changed their name to " << user->name << std::endl;
            }
        }
    };
};

struct SummaryCommand
{
    std::string name;
    ArgumentParser parser;

    explicit SummaryCommand(ArgumentParser& root) : name("summary"), parser(name)
    {
        this->parser.add_description("summarize the rounds in a demo and print a scoreboard");
        this->parser.add_argument("demo");
        root.add_subparser(this->parser);
    }

    [[nodiscard]] int main() const
    {
        Timer timer;
        std::string path = this->parser.get("demo");
        if (!std::filesystem::exists(path))
        {
            std::cerr << "No such file " << path << std::endl;
            return -1;
        }

        std::ifstream file_stream(path, std::ios::binary);
        IstreamInputStream file_input_stream(&file_stream);
        CodedInputStream coded_input_stream(&file_input_stream);

        try
        {
            Client<SummaryObserver> client(coded_input_stream);
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
