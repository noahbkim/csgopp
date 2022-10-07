#include "send_table.h"
#include "../common/macro.h"
#include "../demo.h"

namespace csgopp::network
{

SendTable::Value SendTable::Property::deserialize(CodedInputStream& stream)
{
    switch (this->type)
    {
        case Type::INT32:
            return this->deserialize_int32(stream);
        case Type::FLOAT:
            return this->deserialize_float(stream);
        case Type::VECTOR2:
            return this->deserialize_vector2(stream);
        case Type::VECTOR3:
            return this->deserialize_vector3(stream);
        case Type::STRING:
            return this->deserialize_string(stream);
        case Type::ARRAY:
            return this->deserialize_array(stream);
        default:
            throw GameError("unexpected property type: " + std::string(describe_send_table_property_type(this->type)));
    }
}

inline SendTable::Value SendTable::Property::deserialize_float(CodedInputStream& stream)
{
//    if ((this->flags & Flag::Special) == 0)
//    {
//
//    }
    return SendTable::Value(0.0f);
}

inline SendTable::Value SendTable::Property::deserialize_float_bit_coordinates(CodedInputStream& stream)
{
    return SendTable::Value(0.0f);
}

inline SendTable::Value SendTable::Property::deserialize_float_bit_normal(CodedInputStream& stream)
{
    return SendTable::Value(0.0f);
}

inline SendTable::Value SendTable::Property::deserialize_float_bit_cell_coordinates(CodedInputStream& stream)
{
    return SendTable::Value(0.0f);
}

inline SendTable::Value SendTable::Property::deserialize_int32(CodedInputStream& stream)
{
    return SendTable::Value(42);
}

inline SendTable::Value SendTable::Property::deserialize_vector2(CodedInputStream& stream)
{
    return SendTable::Value(Vector3 {0.0f, 0.0f, 0.0f});
}

inline SendTable::Value SendTable::Property::deserialize_vector3(CodedInputStream& stream)
{
    return SendTable::Value(Vector3 {0.0f, 0.0f, 0.0f});
}

inline SendTable::Value SendTable::Property::deserialize_array(CodedInputStream& stream)
{
    return SendTable::Value("");
}

inline SendTable::Value SendTable::Property::deserialize_string(CodedInputStream& stream)
{
    return SendTable::Value("");
}

SendTable::SendTable(CodedInputStream& stream)
{
    this->deserialize(stream);
}

bool SendTable::deserialize(CodedInputStream& stream)
{
    CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
    OK(limit > 0);

    csgo::message::net::CSVCMsg_SendTable data;
    data.ParseFromCodedStream(&stream);

    this->deserialize(data);

    OK(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
    return data.is_end();
}

void SendTable::deserialize(csgo::message::net::CSVCMsg_SendTable& data)
{
    this->name = data.net_table_name();

    using csgo::message::net::CSVCMsg_SendTable_sendprop_t;
    for (const CSVCMsg_SendTable_sendprop_t& property_data : data.props())
    {
        SendTable::Property& property = this->properties.emplace_back();
        property.type = property_data.type();
        property.data_table_name = property_data.dt_name();
        property.flags = property_data.flags();
        property.elements_count = property_data.num_elements();
        property.high_value = property_data.high_value();
        property.low_value = property_data.low_value();
        property.bits_count = property_data.num_bits();
    }
}

}
