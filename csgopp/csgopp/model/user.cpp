#include "user.h"

namespace csgopp::model::user
{
using csgopp::common::reader::ContainerReader;
using csgopp::common::reader::BigEndian;
using csgopp::common::reader::LittleEndian;

void User::deserialize(const std::string& data)
{
    ContainerReader<std::string> reader(data);
    this->version = reader.deserialize<uint64_t, BigEndian>();
    this->xuid = reader.deserialize<uint64_t, BigEndian>();
    this->name = reader.terminated();
    this->user_id = reader.deserialize<uint32_t, BigEndian>();
    this->guid = reader.terminated();
    this->friends_id = reader.deserialize<uint32_t, BigEndian>();
    this->friends_name = reader.terminated();
    this->is_fake = reader.deserialize<uint8_t>() != 0;
    this->is_hltv = reader.deserialize<uint8_t>() != 0;
    this->custom_files[0] = reader.deserialize<uint32_t, LittleEndian>();
    this->custom_files[1] = reader.deserialize<uint32_t, LittleEndian>();
    this->custom_files[2] = reader.deserialize<uint32_t, LittleEndian>();
    this->custom_files[3] = reader.deserialize<uint32_t, LittleEndian>();
}

}
