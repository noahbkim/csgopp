#include "network.h"
#include "common/magic.h"

namespace csgopp::network
{

//template<>
//SendTable* Network::deserialize(CodedInputStream& stream)
//{
//    CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
//    OK(limit > 0);
//
//    csgo::message::net::CSVCMsg_SendTable data;
//    data.ParseFromCodedStream(&stream);
//
//    this->deserialize(data);
//
//    OK(stream.BytesUntilLimit() == 0);
//    stream.PopLimit(limit);
//    return data.is_end();
//}

}
