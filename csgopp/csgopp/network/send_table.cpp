#include "send_table.h"
#include "../demo.h"

namespace csgopp::network
{

std::unique_ptr<SendTable::Property> SendTable::Property::deserialize(
    SendTable* send_table,
    const CSVCMsg_SendTable_sendprop_t& data
) {
    switch (deserialize_send_table_property_type(data.type()))
    {
        case Type::INT32: return std::make_unique<Int32Property>(send_table, data);
        case Type::FLOAT: return std::make_unique<FloatProperty>(send_table, data);
        case Type::VECTOR3: return std::make_unique<Vector3Property>(send_table, data);
        case Type::VECTOR2: return std::make_unique<Vector2Property>(send_table, data);
        case Type::STRING: return std::make_unique<StringProperty>(send_table, data);
        case Type::ARRAY: return std::make_unique<ArrayProperty>(send_table, data);
        case Type::DATA_TABLE: return std::make_unique<DataTableProperty>(send_table, data);
        case Type::INT64: return std::make_unique<Int64Property>(send_table, data);
        default: throw csgopp::error::GameError("unreachable");
    }
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
        this->properties.emplace_back(Property::deserialize(this, property_data));
    }
}

}
