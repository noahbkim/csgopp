#pragma once

#include <vector>
#include <string>
#include <absl/container/flat_hash_map.h>

#include "netmessages.pb.h"
#include "../common/lookup.h"
#include "../common/object.h"
#include "../common/database.h"

namespace csgopp::client::game_event
{

using common::object::DefaultValueType;
using common::object::Type;
using common::object::ObjectType;
using common::object::Instance;
using common::object::shared;
using common::code::Cursor;
using common::code::Definition;
using common::code::Declaration;
using common::database::DatabaseWithName;
using common::database::Delete;

struct StringType : public DefaultValueType<std::string>
{
    void emit(Cursor<Declaration>& cursor) const override;
};

struct FloatType : public DefaultValueType<float>
{
    void emit(Cursor<Declaration>& cursor) const override;
};

struct LongType : public DefaultValueType<int32_t>
{
    void emit(Cursor<Declaration>& cursor) const override;
};

struct ShortType : public DefaultValueType<int16_t>
{
    void emit(Cursor<Declaration>& cursor) const override;
};

struct ByteType : public DefaultValueType<uint8_t>
{
    void emit(Cursor<Declaration>& cursor) const override;
};

struct BoolType : public DefaultValueType<bool>
{
    void emit(Cursor<Declaration>& cursor) const override;
};

struct UnsignedInt64Type : public DefaultValueType<uint64_t>
{
    void emit(Cursor<Declaration>& cursor) const override;
};

struct WideStringType : public DefaultValueType<std::wstring>
{
    void emit(Cursor<Declaration>& cursor) const override;
};

struct GameEventType final : public ObjectType
{
    using Id = int32_t;

    Id id;
    std::string name;

    GameEventType(Builder&& builder, Id id, std::string&& name);

    static GameEventType* build(csgo::message::net::CSVCMsg_GameEventList_descriptor_t&& descriptor);
};

std::shared_ptr<Type> lookup_type(int32_t type);

struct GameEvent final : public Instance<GameEventType>
{
    using Id = GameEventType::Id;

};

using GameEventTypeDatabase = DatabaseWithName<GameEventType, Delete<GameEventType>>;

}
