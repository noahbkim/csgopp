#include "demo.hpp"

#include <stdexcept>
#include <string>

namespace csgopp::demo
{

/// Parse a single Event from the front of an istream&.
Event parse(std::istream& input)
{
    char command;
    if (!input.get(command))
    {
        throw std::runtime_error("encountered unexpected EOF while parsing demo!");
    }

    switch (command)
    {
        case static_cast<char>(Event::Type::SIGN_ON):
            break;
        case static_cast<char>(Event::Type::PACKET):
            break;
        case static_cast<char>(Event::Type::SYNC_TICK):
            break;
        case static_cast<char>(Event::Type::CONSOLE_COMMAND):
            break;
        case static_cast<char>(Event::Type::USER_COMMAND):
            break;
        case static_cast<char>(Event::Type::DATA_TABLES):
            break;
        case static_cast<char>(Event::Type::STOP):
            return Event(Event::Stop {});
        case static_cast<char>(Event::Type::CUSTOM_DATA):
            break;
        case static_cast<char>(Event::Type::STRING_TABLES):
            break;
        default:
            throw std::domain_error("encountered unknown command " + std::to_string(command));
    }
}

}