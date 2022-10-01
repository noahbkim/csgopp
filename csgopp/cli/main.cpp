#include <iostream>
#include <fstream>

#include "demo.hpp"

using namespace csgopp::demo;

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "must pass a demo" << std::endl;
        return -1;
    }

    std::ifstream input(argv[1]);
    Header header = parse_header(input);

    std::cout << header.magic << std::endl;
    std::cout << header.demo_protocol << std::endl;
    std::cout << header.network_protocol << std::endl;
    std::cout << header.server_name << std::endl;
    std::cout << header.client_name << std::endl;
    std::cout << header.map_name << std::endl;
    std::cout << header.game_directory << std::endl;
    std::cout << header.playback_time << std::endl;
    std::cout << header.tick_count << std::endl;
    std::cout << header.frame_count << std::endl;
    std::cout << header.sign_on_size << std::endl;

    return -1;
}
