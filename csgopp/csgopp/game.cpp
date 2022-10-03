#include "game.h"

namespace csgopp::game
{

using common::reader::LittleEndian;

Game::Game(demo::Header&& header) : _header(header)
{

}

Game::Game(Reader& reader) : Game(demo::Header::deserialize(reader)) {}

bool Game::advance(Reader& reader)
{
    char command = reader.read<char>();
    reader.skip(1);  // player slot
    this->_tick = reader.read<int32_t, LittleEndian>();

    switch (command)
    {
        case static_cast<char>(demo::Command::SIGN_ON):
            printf("sign_on\n");
            reader.skip(152 + 4 + 4);
            reader.skip(reader.read<int32_t, LittleEndian>());
            break;
        case static_cast<char>(demo::Command::PACKET):
            printf("packet\n");
            reader.skip(152 + 4 + 4);
            reader.skip(reader.read<int32_t, LittleEndian>());
            break; // parse_packet(input);;
        case static_cast<char>(demo::Command::SYNC_TICK):
            printf("sync_tick\n");
            break;
        case static_cast<char>(demo::Command::CONSOLE_COMMAND):
            printf("console_command\n");
            reader.skip(reader.read<int32_t, LittleEndian>());
            break; // parse_console_command(input);
        case static_cast<char>(demo::Command::USER_COMMAND):
            printf("user_command\n");
            reader.skip(4);
            reader.skip(reader.read<int32_t, LittleEndian>());
            break; // parse_user_command(input);
        case static_cast<char>(demo::Command::DATA_TABLES):
            printf("data_tables\n");
            reader.skip(reader.read<int32_t, LittleEndian>());
            break; // parse_data_tables(input);
        case static_cast<char>(demo::Command::STOP):
            printf("stop\n");
            break;
        case static_cast<char>(demo::Command::CUSTOM_DATA):
            throw demo::ParseError("encountered unexpected CUSTOM_DATA event!");
        case static_cast<char>(demo::Command::STRING_TABLES):
            printf("string_tables\n");
            reader.skip(reader.read<int32_t, LittleEndian>());
            break; // parse_string_tables(input);
        default:
            throw demo::ParseError("encountered unknown command " + std::to_string(command));
    }

    this->on_frame();
    return command != static_cast<char>(demo::Command::STOP);
}

}
