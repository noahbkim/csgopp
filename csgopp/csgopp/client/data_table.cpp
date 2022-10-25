#include "data_table.h"

namespace csgopp::client::data_table
{

DataTable::Property::Property(CSVCMsg_SendTable_sendprop_t&& data)
    : name(std::move(*data.mutable_var_name()))
    , flags(data.flags())
    , priority(data.priority())
{}

DataTable::Int32Property::Int32Property(CSVCMsg_SendTable_sendprop_t&& data)
    : bits(data.num_bits())
    , Property(std::move(data))
{}

DataTable::Property::Type::T DataTable::Int32Property::type() const
{
    return Type::INT32;
}

DataTable::FloatProperty::FloatProperty(CSVCMsg_SendTable_sendprop_t&& data)
    : high_value(data.high_value())
    , low_value(data.low_value())
    , bits(data.num_bits())
    , Property(std::move(data))
{}

DataTable::Property::Type::T DataTable::FloatProperty::type() const
{
    return Type::FLOAT;
}

DataTable::Property::Type::T DataTable::Vector3Property::type() const
{
    return Type::VECTOR3;
}

DataTable::Property::Type::T DataTable::Vector2Property::type() const
{
    return Type::VECTOR2;
}

DataTable::Property::Type::T DataTable::StringProperty::type() const
{
    return Type::STRING;
}

DataTable::ArrayProperty::ArrayProperty(CSVCMsg_SendTable_sendprop_t&& data, Property* element)
    : element(std::move(element))
    , size(data.num_elements())
    , Property(std::move(data))
{}

DataTable::Property::Type::T DataTable::ArrayProperty::type() const
{
    return Type::ARRAY;
}

DataTable::Property::Type::T DataTable::DataTableProperty::type() const
{
    return Type::DATA_TABLE;
}

DataTable::Int64Property::Int64Property(CSVCMsg_SendTable_sendprop_t&& data)
    : bits(data.num_bits())
    , Property(std::move(data))
{}

DataTable::Property::Type::T DataTable::Int64Property::type() const
{
    return Type::INT64;
}

}
