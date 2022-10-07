#include "server_class.h"
#include "../demo.h"

namespace csgopp::network
{
    ServerClass::ServerClass(CodedInputStream& stream)
    {
        this->deserialize(stream);
    }

    void ServerClass::deserialize(CodedInputStream& stream)
    {
        OK(csgopp::demo::ReadLittleEndian16(stream, &this->id));
        OK(csgopp::demo::ReadCStyleString(stream, &this->name));
        OK(csgopp::demo::ReadCStyleString(stream, &this->data_table_name));
        // todo: determine data_tables index
        // todo: link baseline
    }
}
