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
using csgopp::error::GameError;

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
    CASE(Command::STRING_TABLES, "STRING_TABLES"));

LOOKUP(describe_net_message, int32_t, const char*,
    CASE(0, "net_NOP")
    CASE(1, "net_Disconnect")
    CASE(2, "net_File")
    CASE(3, "net_SplitScreenUser")
    CASE(4, "net_Tick")
    CASE(5, "net_StringCmd")
    CASE(6, "net_SetConVar")
    CASE(7, "net_SignonState")
    CASE(8, "svc_ServerInfo")
    CASE(9, "svc_SendTable")
    CASE(10, "svc_ClassInfo")
    CASE(11, "svc_SetPause")
    CASE(12, "svc_CreateStringTable")
    CASE(13, "svc_UpdateStringTable")
    CASE(14, "svc_VoiceInit")
    CASE(15, "svc_VoiceData")
    CASE(16, "svc_Print")
    CASE(17, "svc_Sounds")
    CASE(18, "svc_SetView")
    CASE(19, "svc_FixAngle")
    CASE(20, "svc_CrosshairAngle")
    CASE(21, "svc_BSPDecal")
    CASE(22, "svc_SplitScreen")
    CASE(23, "svc_UserMessage")
    CASE(24, "svc_EntityMessage")
    CASE(25, "svc_GameEvent")
    CASE(26, "svc_PacketEntities")
    CASE(27, "svc_TempEntities")
    CASE(28, "svc_Prefetch")
    CASE(29, "svc_Menu")
    CASE(30, "svc_GameEventList")
    CASE(31, "svc_GetCvarValue")
    CASE(33, "svc_PaintmapData")
    CASE(34, "svc_CmdKeyValues")
    CASE(35, "svc_EncryptedData")
    CASE(36, "svc_HltvReplay")
    CASE(38, "svc_Broadcast_Command")
    CASE(100, "net_PlayerAvatarData")
    DEFAULT(throw GameError("unknown net message: " + std::to_string(key))));

}
