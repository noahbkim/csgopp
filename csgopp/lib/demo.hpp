#pragma once

#include <istream>
#include <cstdint>

namespace csgopp::demo
{

class ParseError : std::domain_error
{
    using std::domain_error::domain_error;
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
};

Header parse_header(std::istream& input);

struct Frame
{
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

    struct SignOn {};

    struct Packet
    {

    };

    struct SyncTick {};

    struct ConsoleCommand
    {

    };

    struct UserCommand
    {

    };

    struct DataTables
    {

    };

    struct Stop {};

    struct CustomData
    {

    };

    struct StringTables
    {

    };

    union Data
    {
        SignOn sign_on;
        Packet packet;
        SyncTick sync_tick;
        ConsoleCommand console_command;
        UserCommand user_command;
        DataTables data_tables;
        Stop stop;
        CustomData custom_data;
        StringTables string_tables;

        explicit Data(SignOn data) noexcept : sign_on(data) {}
        explicit Data(Packet data) noexcept : packet(data) {}
        explicit Data(SyncTick data) noexcept : sync_tick(data) {}
        explicit Data(ConsoleCommand data) noexcept : console_command(data) {}
        explicit Data(UserCommand data) noexcept : user_command(data) {}
        explicit Data(DataTables data) noexcept : data_tables(data) {}
        explicit Data(Stop data) noexcept : stop(data) {}
        explicit Data(CustomData data) noexcept : custom_data(data) {}
        explicit Data(StringTables data) noexcept : string_tables(data) {}
    };

    Command command;
    Data data;

    explicit Frame(SignOn data) noexcept : command(Command::SIGN_ON), data(data) {}
    explicit Frame(Packet data) noexcept : command(Command::PACKET), data(data) {}
    explicit Frame(SyncTick data) noexcept : command(Command::SYNC_TICK), data(data) {}
    explicit Frame(ConsoleCommand data) noexcept : command(Command::CONSOLE_COMMAND), data(data) {}
    explicit Frame(UserCommand data) noexcept : command(Command::USER_COMMAND), data(data) {}
    explicit Frame(DataTables data) noexcept : command(Command::DATA_TABLES), data(data) {}
    explicit Frame(Stop data) noexcept : command(Command::STOP), data(data) {}
    explicit Frame(CustomData data) noexcept : command(Command::CUSTOM_DATA), data(data) {}
    explicit Frame(StringTables data) noexcept : command(Command::STRING_TABLES), data(data) {}
};

Frame parse_frame(std::istream& input);

template<typename T>
T parse_little_endian(std::istream& input)
{
//    static_assert(sizeof(char) == sizeof(uint8_t));
//    static_assert(sizeof(T) > 1);

    T result {};
    uint8_t buffer[sizeof(T)];
    input.read(reinterpret_cast<char*>(buffer), sizeof(T));
    for (uint32_t i{0}; i < sizeof(T); ++i)
    {
        result += static_cast<T>(buffer[i]) << (i * 8);
    }

    return result;
}

template<typename T>
T parse_variable_size(std::istream& input)
{
//    static_assert(sizeof(char) == sizeof(uint8_t));
//    static_assert(sizeof(T) > 1);

    T result {};
    uint8_t cursor = 0x80;
    for (uint32_t i{0}; (cursor & 0x80) != 0 && i < sizeof(T); ++i)
    {
        input.read(reinterpret_cast<char*>(&cursor), 1);
        result |= static_cast<T>(cursor & 0x7F) << (7 * i);
    }

    return result;
}

}
