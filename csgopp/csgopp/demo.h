#pragma once

#include <istream>
#include <cstdint>

#include "error.h"
#include "common/reader.h"
#include "common/lookup.h"

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

template <typename T, typename S = size_t>
struct VariableSize
{
    static_assert(sizeof(char) == sizeof(uint8_t));
    static_assert(sizeof(T) > 1);

    T value{0};
    S size{0};

    static constexpr size_t limit()
    {
        return (sizeof(T) * 8 + 6) / 7;
    }

    static VariableSize deserialize(Reader& reader)
    {
        VariableSize result;
        std::byte cursor;

        do
        {
            reader.read(&cursor, 1);
            result.value |= static_cast<T>(cursor & std::byte{0x7F}) << (7 * result.size);
            result.size += 1;
        } while ((cursor & std::byte{0x80}) != std::byte{0x00} && result.size <= limit());
        return result;
    }
};

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

LOOKUP(describe, Command, const char*,
    CASE(Command::SIGN_ON, "SIGN_ON")
    CASE(Command::PACKET, "PACKET")
    CASE(Command::SYNC_TICK, "SYNC_TICK")
    CASE(Command::CONSOLE_COMMAND, "CONSOLE_COMMAND")
    CASE(Command::USER_COMMAND, "USER_COMMAND")
    CASE(Command::DATA_TABLES, "DATA_TABLES")
    CASE(Command::STOP, "STOP")
    CASE(Command::CUSTOM_DATA, "CUSTOM_DATA")
    CASE(Command::STRING_TABLES, "STRING_TABLES"))

}
