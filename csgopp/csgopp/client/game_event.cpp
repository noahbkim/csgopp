#include "game_event.h"
#include "../error.h"

namespace csgopp::client::game_event
{

using csgopp::error::GameError;
using csgopp::common::object::Type;

void StringType::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "std::string";
}

void FloatType::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "float";
}

void LongType::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "int32_t";
}

void ShortType::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "int16_t";
}

void ByteType::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "uint8_t";
}

void BoolType::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "bool";
}

void UnsignedInt64Type::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "uint64_t";
}

void WideStringType::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "std::wstring";
}

std::shared_ptr<Type> lookup_type(int32_t type)
{
    switch (type)
    {
        case 1: return shared<StringType>();
        case 2: return shared<FloatType>();
        case 3: return shared<LongType>();
        case 4: return shared<ShortType>();
        case 5: return shared<ByteType>();
        case 6: return shared<BoolType>();
        case 7: return shared<UnsignedInt64Type>();
        case 8: return shared<WideStringType>();
        default: throw GameError("invalid game event value type " + std::to_string(type));
    }
}

GameEventType::GameEventType(Builder&& builder, Id id, std::string&& name)
    : ObjectType(std::move(builder))
    , id(id)
    , name(std::move(name))
{}

GameEventType* GameEventType::build(csgo::message::net::CSVCMsg_GameEventList_descriptor_t&& descriptor)
{
    ObjectType::Builder builder;
    builder.name = descriptor.name();

    for (csgo::message::net::CSVCMsg_GameEventList_key_t& key : *descriptor.mutable_keys())
    {
        builder.member(std::move(*key.mutable_name()), lookup_type(key.type()));
    }

    return new GameEventType(std::move(builder), descriptor.eventid(), std::move(*descriptor.mutable_name()));
}

}