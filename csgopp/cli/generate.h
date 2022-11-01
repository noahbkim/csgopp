#pragma once

#include <iostream>
#include <fstream>

#include <google/protobuf/io/coded_stream.h>
#include <argparse/argparse.hpp>

#include <csgopp/common/code.h>
#include <csgopp/client/data_table.h>
#include <csgopp/client/server_class.h>
#include <csgopp/client.h>

#include "common.h"

using argparse::ArgumentParser;
using csgopp::common::code::Generator;
using csgopp::client::ServerClass;
using csgopp::client::DataTable;
using csgopp::client::StringTable;
using csgopp::client::ClientObserverBase;
using csgopp::client::Client;

struct GenerateObserver : public ClientObserverBase<GenerateObserver>
{
    size_t server_class_count = 0;
    size_t data_table_count = 0;
    size_t entity_type_count = 0;
    Generator generator;

    using ClientObserverBase::ClientObserverBase;

    void on_data_table_creation(Client& client, const DataTable* data_table) override
    {
        this->data_table_count += 1;
        if (data_table->entity_type)
        {
            data_table->entity_type->emit(generator.append(data_table->name));
            this->entity_type_count += 1;
        }
    }

    void on_server_class_creation(Client& client, const ServerClass* server_class) override
    {
        this->server_class_count += 1;
    }
};

struct GenerateCommand
{
    std::string name;
    ArgumentParser parser;

    explicit GenerateCommand(ArgumentParser& root) : name("generate"), parser(name)
    {
        this->parser.add_description("generate C++ structs for the server classes and data tables in a given demo");
        this->parser.add_argument("-o", "--output")
            .help("a file name to write the classes to, defaults to the protocol number")
            .implicit_value(false);
        this->parser.add_argument("-i", "--include")
            .help("a series of paths to quote-include at the top of the generated file")
            .nargs(argparse::nargs_pattern::at_least_one)
            .default_value(std::vector<std::string>{});
        this->parser.add_argument("-n", "--namespace")
            .help("enclose the declarations in a namespace based on the network protocol")
            .implicit_value(true)
            .default_value(false);
        this->parser.add_argument("demo");
        root.add_subparser(this->parser);
    }

    int main(ArgumentParser& root) const
    {
        Timer timer;
        std::string path = this->parser.get("demo");
        std::ifstream file_stream(path, std::ios::binary);
        IstreamInputStream file_input_stream(&file_stream);
        CodedInputStream coded_input_stream(&file_input_stream);

        try
        {
            Client<GenerateObserver> client(coded_input_stream);
            std::string network_protocol = std::to_string(client.header().network_protocol);
            std::cout << "found network protocol version " << network_protocol << std::endl;

            std::string output(this->parser.present("-o").value_or(network_protocol + ".h"));

            // Run simulation
            while (client.advance(coded_input_stream));

            std::ofstream out(output);

            bool includes_provided{false};
            for (const std::string& include : this->parser.get<std::vector<std::string>>("-i"))
            {
                out << "#include \"" << include << "\"" << std::endl;
                includes_provided = true;
            }
            if (!includes_provided)
            {
                out << "#include <cstdint>" << std::endl;
                out << "#include <string>" << std::endl;
                out << std::endl;
                out << "struct Vector3 { float x; float y; float z; };" << std::endl;
                out << "struct Vector2 { float x; float y; };" << std::endl;
                out << std::endl;
            }

            if (this->parser.get<bool>("-n"))
            {
                out << "namespace csgo::protocol" << network_protocol << std::endl;
                out << "{" << std::endl;
                out << std::endl;
            }

            client.observer.generator.write(out);

            if (this->parser.get<bool>("-n"))
            {
                out << "}" << std::endl;
            }

            std::cout << "found " << client.observer.data_table_count << " data tables" << std::endl;
            std::cout << "found " << client.observer.server_class_count << " server classes" << std::endl;
            std::cout << "wrote " << client.observer.entity_type_count << " structs" << std::endl;
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
