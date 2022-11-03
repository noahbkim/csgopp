#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>

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

    explicit GenerateObserver(Client& client) : ClientObserverBase<GenerateObserver>(client) {}

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
        this->parser.add_argument("-f", "--file")
            .help("a file name to write the classes to, defaults to the protocol number");
        this->parser.add_argument("-d", "--directory")
            .help("a directory name to write output files to, may be specified with file");
        this->parser.add_argument("-i", "--include")
            .help("a series of paths to quote-include at the top of the generated file")
            .nargs(argparse::nargs_pattern::at_least_one)
            .default_value(std::vector<std::string>{})
            .required();
        this->parser.add_argument("-n", "--namespace")
            .help("enclose the declarations in a namespace based on the network protocol")
            .implicit_value(true)
            .default_value(false);
        this->parser.add_argument("-v", "--verify")
            .help("output extra data files for debugging and verification")
            .implicit_value(true)
            .default_value(false);
        this->parser.add_argument("demo");
        root.add_subparser(this->parser);
    }

    void write_definitions(const std::filesystem::path& path, Client<GenerateObserver>& client) const
    {
        std::ofstream out(path);
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
            out << "namespace csgo::protocol" << client.header().network_protocol << std::endl;
            out << "{" << std::endl;
            out << std::endl;
        }

        client.observer.generator.write(out);

        if (this->parser.get<bool>("-n"))
        {
            out << "}" << std::endl;
        }
    }

    void write_offsets(const std::filesystem::path& path, Client<GenerateObserver>& client) const
    {
        std::ofstream out(path);
        for (const DataTable* data_table : client.data_tables())
        {
            if (data_table->entity_type)
            {
                out << data_table->entity_type->name << std::endl;
                for (const csgopp::client::entity::Offset& absolute : data_table->entity_type->prioritized)
                {
                    out << "  " << absolute.property->name << std::endl;
                }
            }
        }
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
            Client<GenerateObserver> client(coded_input_stream);
            std::string network_protocol = std::to_string(client.header().network_protocol);
            std::cout << "found network protocol version " << network_protocol << std::endl;

            // Run simulation
            while (client.advance(coded_input_stream));

            std::filesystem::path directory(this->parser.present("-d").value_or("."));
            std::filesystem::path file(this->parser.present("-f").value_or(network_protocol + ".h"));
            std::filesystem::path destination = directory / file;

            this->write_definitions(destination, client);
            if (this->parser.get<bool>("-v"))
            {
                this->write_offsets(destination.replace_extension("offsets.txt"), client);
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
