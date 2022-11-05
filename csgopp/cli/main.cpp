#include <iostream>
#include <stdexcept>

#include <argparse/argparse.hpp>

#include "generate.h"
#include "advance.h"

using argparse::ArgumentParser;

const char* HELP = " Use -h to see help.";

int main(int argc, char** argv)
{
    ArgumentParser parser("csgopp.cli");
    GenerateCommand generate(parser);
    AdvanceCommand advance(parser);

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
        return generate.main();
    }
    else if (parser.is_subcommand_used(advance.name))
    {
        return advance.main();
    }
    else
    {
        std::cerr << "Expected a subcommand." << HELP << std::endl;
    }
}
