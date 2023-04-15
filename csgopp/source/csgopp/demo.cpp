#include "csgopp/demo.h"
#include "netmessages.pb.h"

namespace csgopp::demo
{

bool read_little_endian_uint16(CodedInputStream& stream, uint16_t* value)
{
    uint8_t buffer[2];
    if (!stream.ReadRaw(buffer, 2))
    {
        return false;
    }

    *value = (static_cast<uint16_t>(buffer[1]) << 8) | buffer[0];
    return true;
}

// When in Rome
bool read_c_style_string(CodedInputStream& stream, std::string* string)
{
    do
    {
        string->push_back(0);
        if (!stream.ReadRaw(&string->back(), 1))
        {
            return false;
        }
    } while (string->back() != 0);
    string->pop_back();
    return true;
}

Header::Header(CodedInputStream& stream)
{
    this->deserialize(stream);
}

void Header::deserialize(CodedInputStream& stream)
{
    stream.ReadRaw(this->magic, 8);
    stream.ReadLittleEndian32(&this->demo_protocol);
    stream.ReadLittleEndian32(&this->network_protocol);
    stream.ReadRaw(this->server_name, 260);
    stream.ReadRaw(this->client_name, 260);
    stream.ReadRaw(this->map_name, 260);
    stream.ReadRaw(this->game_directory, 260);
    stream.ReadRaw(&this->playback_time, sizeof(float));
    stream.ReadLittleEndian32(&this->tick_count);
    stream.ReadLittleEndian32(&this->frame_count);
    stream.ReadLittleEndian32(&this->sign_on_size);
}

}