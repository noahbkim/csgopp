#include "demo.h"
#include "netmessages.pb.h"

#include "game/server_class.h"

namespace csgopp::demo
{

Header Header::deserialize(Reader& reader)
{
    Header header{};
    reader.read(header.magic, 8);
    header.demo_protocol = reader.read<int, LittleEndian>();
    header.network_protocol = reader.read<int, LittleEndian>();
    reader.read(header.server_name, 260);
    reader.read(header.client_name, 260);
    reader.read(header.map_name, 260);
    reader.read(header.game_directory, 260);
    header.playback_time = reader.read<float>();
    header.tick_count = reader.read<int, LittleEndian>();
    header.frame_count = reader.read<int, LittleEndian>();
    header.sign_on_size = reader.read<int, LittleEndian>();
    return header;
}



//Frame parse_packet(std::istream& input)
//{
//    // Skip header, we don't use it
//    const std::istream::off_type header_size{152 + 4 + 4};
//    input.seekg(1, std::ios_base::cur);
//
//    int size = parse_little_endian<int>(input);
//    input.seekg(size, std::ios_base::cur);
//    return Frame(Frame::Packet{});
//}
//
//Frame parse_console_command(std::istream& input)
//{
//    int size = parse_little_endian<int>(input);
//    input.seekg(size, std::ios_base::cur);
//    return Frame(Frame::ConsoleCommand{});
//}
//
//Frame parse_user_command(std::istream& input)
//{
//    input.seekg(4, std::ios_base::cur);
//    int size = parse_little_endian<int>(input);
//    input.seekg(size, std::ios_base::cur);
//    return Frame(Frame::UserCommand{});
//}
//
//Frame parse_data_tables(std::istream& input)
//{
//    using csgo::message::net::SVC_Messages;
//    int size = parse_variable_size<int>(input);
//    size_t origin = input.tellg();
//
//    // Iterate SVC messages
////    while (true)
////    {
////        auto svc_message = static_cast<SVC_Messages>(parse_variable_size<int>(input));
////        if (svc_message != SVC_Messages::svc_SendTable)
////        {
////            throw ParseError("expected svc_SendTable");
////        }
////
////
////    }
//    input.seekg(size, std::ios_base::cur);
//
//    assert(origin + size == input.tellg());
//    return Frame(Frame::DataTables{});
//}
//
//Frame parse_string_tables(std::istream& input)
//{
//    int size = parse_little_endian<int>(input);
//    input.seekg(size, std::ios_base::cur);
//    return Frame(Frame::StringTables{});
//}
//
//Frame::Command parse_command(std::istream& input)
//{
//    char command;
//    if (!input.get(command))
//    {
//        throw std::runtime_error("encountered unexpected EOF while parsing demo!");
//    }
//
//    switch (command)
//    {
//        case static_cast<char>(Frame::Command::SIGN_ON):
//            return Frame(Frame::SignOn{});
//        case static_cast<char>(Frame::Command::PACKET):
//            return parse_packet(input);;
//        case static_cast<char>(Frame::Command::SYNC_TICK):
//            return Frame(Frame::SyncTick{});
//        case static_cast<char>(Frame::Command::CONSOLE_COMMAND):
//            return parse_console_command(input);
//        case static_cast<char>(Frame::Command::USER_COMMAND):
//            return parse_user_command(input);
//        case static_cast<char>(Frame::Command::DATA_TABLES):
//            return parse_data_tables(input);
//        case static_cast<char>(Frame::Command::STOP):
//            return Frame(Frame::Stop{});
//        case static_cast<char>(Frame::Command::CUSTOM_DATA):
//            return parse_string_tables(input);
//        case static_cast<char>(Frame::Command::STRING_TABLES):
//            throw ParseError("encountered unexpected STRING_TABLES event!");
//        default:
//            throw ParseError("encountered unknown command " + std::to_string(command));
//    }
//}

}