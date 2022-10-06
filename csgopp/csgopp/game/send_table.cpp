#include "send_table.h"
#include "netmessages.pb.h"
#include "../demo.h"

namespace csgopp::game
{

using csgopp::demo::VariableSize;

SendTable::Value SendTable::Property::deserialize(Reader& reader)
{
    switch (this->type)
    {
        case Type::INT32:
            return this->deserialize_int32(reader);
        case Type::FLOAT:
            return this->deserialize_float(reader);
        case Type::VECTOR2:
            return this->deserialize_vector2(reader);
        case Type::VECTOR3:
            return this->deserialize_vector3(reader);
        case Type::STRING:
            return this->deserialize_string(reader);
        case Type::ARRAY:
            return this->deserialize_array(reader);
        default:
            throw GameError("unexpected property type: " + std::string(describe(this->type)));
    }
}

inline SendTable::Value SendTable::Property::deserialize_float(Reader& reader)
{
//    if ((this->flags & Flag::Special) == 0)
//    {
//
//    }
    return SendTable::Value(0.0f);
}

inline SendTable::Value SendTable::Property::deserialize_float_bit_coordinates(Reader& reader)
{
    return SendTable::Value(0.0f);
}

inline SendTable::Value SendTable::Property::deserialize_float_bit_normal(Reader& reader)
{
    return SendTable::Value(0.0f);
}

inline SendTable::Value SendTable::Property::deserialize_float_bit_cell_coordinates(Reader& reader)
{
    return SendTable::Value(0.0f);
}

inline SendTable::Value SendTable::Property::deserialize_int32(Reader& reader)
{
    return SendTable::Value(42);
}

inline SendTable::Value SendTable::Property::deserialize_vector2(Reader& reader)
{
    return SendTable::Value(Vector3 {0.0f, 0.0f, 0.0f});
}

inline SendTable::Value SendTable::Property::deserialize_vector3(Reader& reader)
{
    return SendTable::Value(Vector3 {0.0f, 0.0f, 0.0f});
}

inline SendTable::Value SendTable::Property::deserialize_array(Reader& reader)
{
    return SendTable::Value("");
}

inline SendTable::Value SendTable::Property::deserialize_string(Reader& reader)
{
    return SendTable::Value("");
}

SendTable SendTable::deserialize(Reader& reader)
{
    VariableSize<int32_t, int32_t> type = VariableSize<int32_t, int32_t>::deserialize(reader);
    csgo::message::net::CSVCMsg_SendTable data;
    return SendTable();
}

}
