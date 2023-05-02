#pragma once

#include <vector>
#include <string>
#include <absl/container/flat_hash_map.h>

#include "../common/database.h"
#include "../common/lookup.h"
#include "netmessages.pb.h"
#include <objective/code.h>
#include <objective.h>

namespace csgopp::client::game_event
{

using common::database::DatabaseWithNameId;
using objective::code::Metadata;
using objective::code::Declaration;
using objective::Instance;
using objective::ObjectType;
using objective::ValueType;
using objective::WrapperType;
using objective::Type;

struct DataType
{
    virtual void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const = 0;
};

struct StringType : public ValueType<std::string>, public DataType
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct FloatType : public ValueType<float>, public DataType
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct LongType : public ValueType<int32_t>, public DataType
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct ShortType : public ValueType<int16_t>, public DataType
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct ByteType : public ValueType<uint8_t>, public DataType
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct BoolType : public ValueType<bool>, public DataType
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct UnsignedInt64Type : public ValueType<uint64_t>, public DataType
{
    void emit(Declaration& declaration, Declaration::Member& member) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
//    void format(const char* address, std::ostream& out) const override;
};

struct WideStringType : public ValueType<std::wstring>, public DataType
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
