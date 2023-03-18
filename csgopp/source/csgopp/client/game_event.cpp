#include "csgopp/client/game_event.h"
#include "csgopp/error.h"

// https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/engine/GameEventManager.cpp

namespace csgopp::client::game_event
{

using object::Type;
using csgopp::error::GameError;
using object::make_shared_static;

void StringType::emit(Declaration& declaration, Declaration::Member& member) const
{
    member.type = "std::string";
}

void StringType::update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const
{
    *reinterpret_cast<Value*>(address) = std::move(*key.mutable_val_string());
}

//void StringType::format(const char* address, std::ostream& out) const
//{
//    out << "\"" << *reinterpret_cast<const Value*>(address) << "\"";
//}

void FloatType::emit(Declaration& declaration, Declaration::Member& member) const
{
    member.type = "float";
}

void FloatType::update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const
{
    *reinterpret_cast<Value*>(address) = key.val_float();
}

//void FloatType::format(const char* address, std::ostream& out) const
//{
//    out << *reinterpret_cast<const Value*>(address);
//}

void LongType::emit(Declaration& declaration, Declaration::Member& member) const
{
    member.type = "int32_t";
}

void LongType::update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const
{
    *reinterpret_cast<Value*>(address) = key.val_long();
}

//void LongType::format(const char* address, std::ostream& out) const
//{
//    out << *reinterpret_cast<const Value*>(address);
//}

void ShortType::emit(Declaration& declaration, Declaration::Member& member) const
{
    member.type = "int16_t";
}

void ShortType::update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const
{
    // According to documentation this should be 16 bits
    // https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/engine/GameEventManager.cpp#L33
    *reinterpret_cast<Value*>(address) = static_cast<Value>(key.val_short());
}

//void ShortType::format(const char* address, std::ostream& out) const
//{
//    out << *reinterpret_cast<const Value*>(address);
//}

void ByteType::emit(Declaration& declaration, Declaration::Member& member) const
{
    member.type = "uint8_t";
}

void ByteType::update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const
{
    *reinterpret_cast<Value*>(address) = key.val_byte();
}

//void ByteType::format(const char* address, std::ostream& out) const
//{
//    out << static_cast<uint16_t>(*reinterpret_cast<const Value*>(address));
//}

void BoolType::emit(Declaration& declaration, Declaration::Member& member) const
{
    member.type = "bool";
}

void BoolType::update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const
{
    *reinterpret_cast<Value*>(address) = key.val_bool();
}

//void BoolType::format(const char* address, std::ostream& out) const
//{
//    out << (*reinterpret_cast<const bool*>(address) ? "true" : "false");
//}

void UnsignedInt64Type::emit(Declaration& declaration, Declaration::Member& member) const
{
    member.type = "uint64_t";
}

void UnsignedInt64Type::update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const
{
    *reinterpret_cast<Value*>(address) = key.val_uint64();
}

//void UnsignedInt64Type::format(const char* address, std::ostream& out) const
//{
//    out << *reinterpret_cast<const Value*>(address);
//}

void WideStringType::emit(Declaration& declaration, Declaration::Member& member) const
{
    member.type = "std::wstring";
}

void WideStringType::update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const
{
    auto* value = reinterpret_cast<Value*>(address);
    value->clear();
    value->append(
        reinterpret_cast<const wchar_t*>(key.val_wstring().data()),
        key.val_wstring().size() / sizeof(wchar_t));
}

//void WideStringType::format(const char* address, std::ostream& out) const
//{
//    out << "L\"???\"";  // TODO
//}

std::shared_ptr<Type> lookup_type(int32_t type)
{
    switch (type)
    {
    case 1: return make_shared_static<StringType>();
    case 2: return make_shared_static<FloatType>();
    case 3: return make_shared_static<LongType>();
    case 4: return make_shared_static<ShortType>();
    case 5: return make_shared_static<ByteType>();
    case 6: return make_shared_static<BoolType>();
    case 7: return make_shared_static<UnsignedInt64Type>();
    case 8: return make_shared_static<WideStringType>();
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