#pragma once

#include <vector>
#include <string>
#include <absl/container/flat_hash_map.h>

#include "../common/database.h"
#include "../common/lookup.h"
#include "netmessages.pb.h"
#include <object/code.h>
#include <object/object.h>

namespace csgopp::client::game_event
{

using common::database::DatabaseWithNameId;
using object::code::Cursor;
using object::code::Declaration;
using object::code::Definition;
using object::DefaultValueType;
using object::Instance;
using object::ObjectType;
using object::shared;
using object::Type;

struct GameEventValueType : public virtual Type
{
    virtual void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const = 0;
};

struct StringType : public DefaultValueType<std::string>, public GameEventValueType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
    void format(const char* address, std::ostream& out) const override;
};

struct FloatType : public DefaultValueType<float>, public GameEventValueType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
    void format(const char* address, std::ostream& out) const override;
};

struct LongType : public DefaultValueType<int32_t>, public GameEventValueType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
    void format(const char* address, std::ostream& out) const override;
};

struct ShortType : public DefaultValueType<int16_t>, public GameEventValueType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
    void format(const char* address, std::ostream& out) const override;
};

struct ByteType : public DefaultValueType<uint8_t>, public GameEventValueType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
    void format(const char* address, std::ostream& out) const override;
};

struct BoolType : public DefaultValueType<bool>, public GameEventValueType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
    void format(const char* address, std::ostream& out) const override;
};

struct UnsignedInt64Type : public DefaultValueType<uint64_t>, public GameEventValueType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
    void format(const char* address, std::ostream& out) const override;
};

struct WideStringType : public DefaultValueType<std::wstring>, public GameEventValueType
{
    void emit(Cursor<Declaration>& cursor) const override;
    void update(char* address, csgo::message::net::CSVCMsg_GameEvent_key_t&& key) const override;
    void format(const char* address, std::ostream& out) const override;
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
