#pragma once

#include <istream>
#include <cstdint>

namespace csgopp::demo
{

struct Event
{
    enum class Type : uint8_t
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

    struct SignOn
    {

    };

    struct Packet
    {

    };

    struct SyncTick
    {

    };

    struct ConsoleCommand
    {

    };

    struct UserCommand
    {

    };

    struct DataTables
    {

    };

    struct Stop
    {

    };

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

    Type type;
    Data data;

    explicit Event(SignOn data) noexcept : type(Type::SIGN_ON), data(data) {}
    explicit Event(Packet data) noexcept : type(Type::PACKET), data(data) {}
    explicit Event(SyncTick data) noexcept : type(Type::SYNC_TICK), data(data) {}
    explicit Event(ConsoleCommand data) noexcept : type(Type::CONSOLE_COMMAND), data(data) {}
    explicit Event(UserCommand data) noexcept : type(Type::USER_COMMAND), data(data) {}
    explicit Event(DataTables data) noexcept : type(Type::DATA_TABLES), data(data) {}
    explicit Event(Stop data) noexcept : type(Type::STOP), data(data) {}
    explicit Event(CustomData data) noexcept : type(Type::CUSTOM_DATA), data(data) {}
    explicit Event(StringTables data) noexcept : type(Type::STRING_TABLES), data(data) {}
};

Event parse(std::istream& input);

template<typename T>
T parse_little_endian(std::istream &input)
{
    static_assert(sizeof(char) == sizeof(uint8_t));
    static_assert(sizeof(T) > 1);

    uint8_t buffer[sizeof(T)];
    input.read(reinterpret_cast<char *>(buffer), sizeof(T));

    T result {};
    for (uint32_t i{0}; i < sizeof(T); ++i)
    {
        result += static_cast<T>(buffer[i]) << (i * 8);
    }

    return result;
}


}
