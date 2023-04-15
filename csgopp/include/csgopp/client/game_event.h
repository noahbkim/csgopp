#pragma once

#include <vector>
#include <string>
#include <absl/container/flat_hash_map.h>

#include "../common/database.h"
#include "../common/lookup.h"
#include "netmessages.pb.h"
#include <object/code.h>
#include <object.h>

namespace csgopp::client::game_event
{

using common::database::DatabaseWithNameId;
using object::code::Metadata;
using object::code::Declaration;
using object::TrivialValueType;
using object::Instance;
using object::ObjectType;
using object::Type;
using object::WrapperType;

struct DataType : public Type
{
    virtual void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const = 0;
};

struct StringType : public WrapperType<TrivialValueType<std::string>, DataType>
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct FloatType : public WrapperType<TrivialValueType<float>, DataType>
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct LongType : public WrapperType<TrivialValueType<int32_t>, DataType>
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct ShortType : public WrapperType<TrivialValueType<int16_t>, DataType>
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct ByteType : public WrapperType<TrivialValueType<uint8_t>, DataType>
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct BoolType : public WrapperType<TrivialValueType<bool>, DataType>
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct UnsignedInt64Type : public WrapperType<TrivialValueType<uint64_t>, DataType>
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct WideStringType : public WrapperType<TrivialValueType<std::wstring>, DataType>
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct GameEventType final : public ObjectType
{
    using Id = int32_t;

    Id id;
    std::string name;

    GameEventType(Builder&& builder, Id id, std::string&& name);

    static std::shared_ptr<GameEventType> build(csgo::message::net::CSVCMsg_GameEventList_descriptor_t&& descriptor);
};

std::shared_ptr<Type> lookup_type(int32_t type);

struct GameEvent final : public Instance<GameEventType>
{
    using Id = GameEventType::Id;

    Id id;
    using Instance<GameEventType>::Instance;
};

using GameEventTypeDatabase = DatabaseWithNameId<GameEventType>;

}
