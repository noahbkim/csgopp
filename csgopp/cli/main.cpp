#include <iostream>
#include <fstream>

#include <csgopp/demo.h>
#include <csgopp/game.h>

using namespace csgopp::demo;
using csgopp::game::Game;
using csgopp::common::reader::StreamReader;

class CLIGame : public Game
{
public:
    explicit CLIGame(Reader& reader) : Game(reader)
    {

    }
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
        StreamReader<std::ifstream> reader(argv[1]);
        CLIGame game(reader);
        game.advance(reader);
        game.advance(reader);
        game.advance(reader);
        game.advance(reader);
    }
    catch (csgopp::error::Error& error)
    {
        std::cerr << "caught exception: " << error.message() << std::endl;
        return -1;
    }
}
