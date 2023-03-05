#include "game_event.h"
#include "../error.h"

// https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/engine/GameEventManager.cpp

namespace csgopp::client::game_event
{

using object::Type;
using csgopp::error::GameError;

void StringType::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "std::string";
}

void StringType::update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const
{
    *reinterpret_cast<Value*>(address) = std::move(*key.mutable_val_string());
}

void StringType::represent(const char* address, std::ostream& out) const
{
    out << "\"" << *reinterpret_cast<const Value*>(address) << "\"";
}

void FloatType::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "float";
}

void FloatType::update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const
{
    *reinterpret_cast<Value*>(address) = key.val_float();
}

void FloatType::represent(const char* address, std::ostream& out) const
{
    out << *reinterpret_cast<const Value*>(address);
}

void LongType::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "int32_t";
}

void LongType::update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const
{
    *reinterpret_cast<Value*>(address) = key.val_long();
}

void LongType::represent(const char* address, std::ostream& out) const
{
    out << *reinterpret_cast<const Value*>(address);
}

void ShortType::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "int16_t";
}

void ShortType::update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const
{
    // According to documentation this should be 16 bits
    // https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/engine/GameEventManager.cpp#L33
    *reinterpret_cast<Value*>(address) = static_cast<Value>(key.val_short());
}

void ShortType::represent(const char* address, std::ostream& out) const
{
    out << *reinterpret_cast<const Value*>(address);
}

void ByteType::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "uint8_t";
}

void ByteType::update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const
{
    *reinterpret_cast<Value*>(address) = key.val_byte();
}

void ByteType::represent(const char* address, std::ostream& out) const
{
    out << static_cast<uint16_t>(*reinterpret_cast<const Value*>(address));
}

void BoolType::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "bool";
}

void BoolType::update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const
{
    *reinterpret_cast<Value*>(address) = key.val_bool();
}

void BoolType::represent(const char* address, std::ostream& out) const
{
    out << (*reinterpret_cast<const bool*>(address) ? "true" : "false");
}

void UnsignedInt64Type::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "uint64_t";
}

void UnsignedInt64Type::update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const
{
    *reinterpret_cast<Value*>(address) = key.val_uint64();
}

void UnsignedInt64Type::represent(const char* address, std::ostream& out) const
{
    out << *reinterpret_cast<const Value*>(address);
}

void WideStringType::emit(Cursor<Declaration>& cursor) const
{
    cursor.target.type = "std::wstring";
}

void WideStringType::update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const
{
    auto* value = reinterpret_cast<Value*>(address);
    value->clear();
    value->append(
        reinterpret_cast<const wchar_t*>(key.val_wstring().data()),
        key.val_wstring().size() / sizeof(wchar_t));
}

void WideStringType::represent(const char* address, std::ostream& out) const
{
    out << "L\"???\"";  // TODO
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
{
}

std::shared_ptr<GameEventType> GameEventType::build(csgo::message::net::CSVCMsg_GameEventList_descriptor_t&& descriptor)
{
    ObjectType::Builder builder;
    builder.name = descriptor.name();

    for (csgo::message::net::CSVCMsg_GameEventList_key_t& key: *descriptor.mutable_keys())
    {
        builder.member(std::move(*key.mutable_name()), lookup_type(key.type()));
    }

    return std::make_shared<GameEventType>(
        std::move(builder),
        descriptor.eventid(),
        std::move(*descriptor.mutable_name())
    );
}

}