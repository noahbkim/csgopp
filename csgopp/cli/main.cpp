#include <iostream>
#include <stdexcept>

#include <argparse/argparse.hpp>

#include "generate.h"

using argparse::ArgumentParser;

const char* HELP = " Use -h to see help.";

int main(int argc, char** argv)
{
    ArgumentParser parser("csgopp.cli");
    GenerateCommand generate(parser);

    try
    {
        parser.parse_args(argc, argv);
    }
    catch (const std::runtime_error& error)
    {
        std::cerr << error.what() << HELP << std::endl;
        return 1;
    }

    if (parser.is_subcommand_used(generate.name))
    {
        return generate.main(parser);
    }
    else
    {
        std::cerr << "Expected a subcommand." << HELP << std::endl;
    }
}

//    try
//    {
//        std::ifstream file_stream(argv[1], std::ios::binary);
//        IstreamInputStream file_input_stream(&file_stream);
//        CodedInputStream coded_input_stream(&file_input_stream);
//        Client<DataObserver> client(coded_input_stream);
//
//        std::cout << "magic: " << client.header().magic << std::endl;
//        std::cout << "demo_protocol: " << client.header().demo_protocol << std::endl;
//        std::cout << "network_protocol: " << client.header().network_protocol << std::endl;
//        std::cout << "server_name: " << client.header().server_name << std::endl;
//        std::cout << "client_name: " << client.header().client_name << std::endl;
//        std::cout << "map_name: " << client.header().map_name << std::endl;
//        std::cout << "game_directory: " << client.header().game_directory << std::endl;
//        std::cout << "playback_time: " << client.header().playback_time << std::endl;
//        std::cout << "tick_count: " << client.header().tick_count << std::endl;
//        std::cout << "frame_count: " << client.header().frame_count << std::endl;
//        std::cout << "sign_on_size: " << client.header().sign_on_size << std::endl;
//
//        try
//        {
//            std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
//
//            // Actually run
//            while (client.advance(coded_input_stream));
//            // for (size_t i = 0; i < 100; ++i) client.advance(coded_input_stream);
//
//            int64_t elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
//                    std::chrono::system_clock::now() - start).count();
//            std::cout << "finished in " << elapsed << " ms" << std::endl;
//            client.observer.report();
//        }
//        catch (csgopp::error::Error& error)
//        {
//            std::cerr << "[" << client.cursor() << "] caught exception: " << error.message() << std::endl;
//            return -1;
//        }
//    }
//    catch (csgopp::error::Error& error)
//    {
//        std::cerr << "caught exception: " << error.message() << std::endl;
//        return -1;
//    }
