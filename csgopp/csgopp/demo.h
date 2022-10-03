#pragma once

#include <istream>
#include <cstdint>

#include "error.h"
#include "common/reader.h"

namespace csgopp::demo
{

using csgopp::common::reader::Reader;
using csgopp::common::reader::LittleEndian;

class ParseError : public csgopp::error::Error
{
    using Error::Error;
};

struct Header
{
    char magic[8];
    int demo_protocol;
    int network_protocol;
    char server_name[260];
    char client_name[260];
    char map_name[260];
    char game_directory[260];
    float playback_time;
    int tick_count;
    int frame_count;
    int sign_on_size;

    static Header deserialize(Reader& reader);
};

template<typename T>
T parse_variable_size(Reader& reader)
{
    static_assert(sizeof(char) == sizeof(uint8_t));
    static_assert(sizeof(T) > 1);

    T result{};
    uint8_t cursor = 0x80;
    for (uint32_t i{0}; (cursor & 0x80) != 0 && i < sizeof(T); ++i)
    {
        reader.read(&cursor, 1);
        result |= static_cast<T>(cursor & 0x7F) << (7 * i);
    }

    return result;
}

enum class Command : uint8_t
{
    SIGN_ON = 1,
    PACKET = 2,
    SYNC_TICK = 3,
    CONSOLE_COMMAND = 4,
    USER_COMMAND = 5,
    DATA_TABLES = 6,
    STOP = 7,
    CUSTOM_DATA = 8,
    STRING_TABLES = 9,
};

}
