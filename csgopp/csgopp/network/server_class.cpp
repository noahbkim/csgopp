#include "server_class.h"
#include "../demo.h"

namespace csgopp::network
{

ServerClass::ServerClass(CodedInputStream& stream, const Database<SendTable>& database)
{
    this->deserialize(stream, database);
}

void ServerClass::deserialize(CodedInputStream& stream, const Database<SendTable>& database)
{
    OK(csgopp::demo::ReadLittleEndian16(stream, &this->id));
    OK(csgopp::demo::ReadCStyleString(stream, &this->name));

    std::string send_table_name;
    OK(csgopp::demo::ReadCStyleString(stream, &send_table_name));
    auto iterator = database.find(send_table_name);
    if (iterator == database.end())
    {
        throw GameError("server class " + this->name + " expected send table " + send_table_name);
    }
    this->send_table = iterator->second.get();
}

}
