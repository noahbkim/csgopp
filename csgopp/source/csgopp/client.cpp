#include "csgopp/client.h"
#include <objective.h>

#ifdef NDEBUG
#define DEBUG(STATEMENT)
#define NOTE(FMT, ...)
#define VERIFY(CONDITION, ...) OK(CONDITION)
#else
#define DEBUG(STATEMENT) STATEMENT;
#define NOTE(FMT, ...) fprintf(stderr, "  " FMT "\n", __VA_ARGS__);
#define VERIFY(CONDITION, ...) do \
{ \
    if (!(CONDITION)) \
    { \
        fprintf(stderr, WHERE() "\n  condition: " #CONDITION "\n  cursor: %d\n  tick: %d\n", this->cursor(), this->tick()); \
        __VA_ARGS__; \
        throw csgopp::error::GameError("failed assertion " #CONDITION); \
    } \
} while (false)
#endif

namespace csgopp::client
{

using csgo::message::net::CSVCMsg_SendTable_sendprop_t;

constexpr size_t MAX_EDICT_BITS = 11;
constexpr size_t ENTITY_HANDLE_INDEX_MASK = (1 << MAX_EDICT_BITS) - 1;
constexpr size_t ENTITY_HANDLE_SERIAL_NUMBER_BITS = 10;
constexpr size_t ENTITY_HANDLE_BITS = MAX_EDICT_BITS + ENTITY_HANDLE_SERIAL_NUMBER_BITS;
constexpr size_t INVALID_ENTITY_HANDLE = (1 << ENTITY_HANDLE_BITS) - 1;

Client::Client(CodedInputStream& stream) : _header(stream)
{
}

Client::Client(Header header) : _header(header)
{
}

bool Client::advance(CodedInputStream& stream)
{
    bool ok = true;
    this->before_frame();

    char command;
    VERIFY(stream.ReadRaw(&command, 1));
    VERIFY(stream.ReadLittleEndian32(&this->_tick));
    VERIFY(stream.Skip(1));  // player slot

    switch (command)
    {
        case demo::Command::SIGN_ON:
        case demo::Command::PACKET:
            this->advance_packets(stream);
            break;
        case demo::Command::SYNC_TICK:
            break;
        case demo::Command::CONSOLE_COMMAND:
            this->advance_console_command(stream);
            break;
        case demo::Command::USER_COMMAND:
            this->advance_user_command(stream);
            break;
        case demo::Command::DATA_TABLES:
            this->advance_data_tables(stream);
            break;
        case demo::Command::STOP:
            ok = false;
            break;
        case demo::Command::CUSTOM_DATA:
            this->advance_custom_data(stream);
            break;
        case demo::Command::STRING_TABLES:
            this->advance_string_tables(stream);
            break;
        default:
            this->advance_unknown(stream, command);
            break;
    }

    this->_cursor += 1;
    this->on_frame(command);
    return ok;
}

void Client::advance_packets(CodedInputStream& stream)
{
    // Arbitrary player data, seems useless
    VERIFY(stream.Skip(152 + 4 + 4));

    uint32_t size;
    VERIFY(stream.ReadLittleEndian32(&size));
    CodedInputStream::Limit limit = stream.PushLimit(size);

    while (stream.BytesUntilLimit() > 0)
    {
        this->advance_packet(stream);
    }

    VERIFY(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

void Client::advance_packet(CodedInputStream& stream)
{
    uint32_t command = stream.ReadTag();
    this->before_packet(command);

    switch (command)
    {
        using namespace csgo::message::net;
        case NET_Messages::net_NOP:
            this->advance_packet_nop(stream);
            break;
        case NET_Messages::net_Disconnect:
            this->advance_packet_disconnect(stream);
            break;
        case NET_Messages::net_File:
            this->advance_packet_file(stream);
            break;
        case NET_Messages::net_SplitScreenUser:
            this->advance_packet_split_screen_user(stream);
            break;
        case NET_Messages::net_Tick:
            this->advance_packet_tick(stream);
            break;
        case NET_Messages::net_StringCmd:
            this->advance_packet_string_command(stream);
            break;
        case NET_Messages::net_SetConVar:
            this->advance_packet_set_console_variable(stream);
            break;
        case NET_Messages::net_SignonState:
            this->advance_packet_sign_on_state(stream);
            break;
        case SVC_Messages::svc_ServerInfo:
            this->advance_packet_server_info(stream);
            break;
        case SVC_Messages::svc_SendTable:
            this->advance_packet_send_table(stream);
            break;
        case SVC_Messages::svc_ClassInfo:
            this->advance_packet_class_info(stream);
            break;
        case SVC_Messages::svc_SetPause:
            this->advance_packet_set_pause(stream);
            break;
        case SVC_Messages::svc_CreateStringTable:
            this->advance_packet_create_string_table(stream);
            break;
        case SVC_Messages::svc_UpdateStringTable:
            this->advance_packet_update_string_table(stream);
            break;
        case SVC_Messages::svc_VoiceInit:
            this->advance_packet_voice_initialization(stream);
            break;
        case SVC_Messages::svc_VoiceData:
            this->advance_packet_voice_data(stream);
            break;
        case SVC_Messages::svc_Print:
            this->advance_packet_print(stream);
            break;
        case SVC_Messages::svc_Sounds:
            this->advance_packet_sounds(stream);
            break;
        case SVC_Messages::svc_SetView:
            this->advance_packet_set_view(stream);
            break;
        case SVC_Messages::svc_FixAngle:
            this->advance_packet_fix_angle(stream);
            break;
        case SVC_Messages::svc_CrosshairAngle:
            this->advance_packet_crosshair_angle(stream);
            break;
        case SVC_Messages::svc_BSPDecal:
            this->advance_packet_bsp_decal(stream);
            break;
        case SVC_Messages::svc_SplitScreen:
            this->advance_packet_split_screen(stream);
            break;
        case SVC_Messages::svc_UserMessage:
            this->advance_packet_user_message(stream);
            break;
        case SVC_Messages::svc_EntityMessage:
            this->advance_packet_entity_message(stream);
            break;
        case SVC_Messages::svc_GameEvent:
            this->advance_packet_game_event(stream);
            break;
        case SVC_Messages::svc_PacketEntities:
            this->advance_packet_packet_entities(stream);
            break;
        case SVC_Messages::svc_TempEntities:
            this->advance_packet_temporary_entities(stream);
            break;
        case SVC_Messages::svc_Prefetch:
            this->advance_packet_prefetch(stream);
            break;
        case SVC_Messages::svc_Menu:
            this->advance_packet_menu(stream);
            break;
        case SVC_Messages::svc_GameEventList:
            this->advance_packet_game_event_list(stream);
            break;
        case SVC_Messages::svc_GetCvarValue:
            this->advance_packet_get_console_variable_value(stream);
            break;
        case SVC_Messages::svc_PaintmapData:
            this->advance_packet_paintmap_data(stream);
            break;
        case SVC_Messages::svc_CmdKeyValues:
            this->advance_packet_command_key_values(stream);
            break;
        case SVC_Messages::svc_EncryptedData:
            this->advance_packet_encrypted_data(stream);
            break;
        case SVC_Messages::svc_HltvReplay:
            this->advance_packet_hltv_replay(stream);
            break;
        case SVC_Messages::svc_Broadcast_Command:
            this->advance_packet_broadcast_command(stream);
            break;
        case NET_Messages::net_PlayerAvatarData:
            this->advance_packet_player_avatar_data(stream);
            break;
        default:
            this->advance_packet_unknown(stream, command);
    }

    this->on_packet(command);
}

void Client::advance_console_command(CodedInputStream& stream)
{
    uint32_t size;
    VERIFY(stream.ReadLittleEndian32(&size));
    VERIFY(stream.Skip(size));
}

void Client::advance_user_command(CodedInputStream& stream)
{
    VERIFY(stream.Skip(4));
    uint32_t size;
    VERIFY(stream.ReadLittleEndian32(&size));
    VERIFY(stream.Skip(size));
}

void Client::advance_data_tables(CodedInputStream& stream)
{
    uint32_t size;
    VERIFY(stream.ReadLittleEndian32(&size));
    CodedInputStream::Limit limit = stream.PushLimit(size);

    this->create_data_tables_and_server_classes(stream);

    VERIFY(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

/// \see https://github.com/markus-wa/demoinfocs-golang/blob/50f55785b7a0ba89164662a000e00cd55969f7ae/pkg/demoinfocs/stringtables.go#L66
/// \todo actually test this method
void Client::advance_string_tables(CodedInputStream& stream)
{
    uint32_t size;
    VERIFY(stream.ReadLittleEndian32(&size));
    CodedInputStream::Limit limit = stream.PushLimit(size);

    DEBUG(int current_position = stream.CurrentPosition());
    char* raw_data = new char[size];
    stream.ReadRaw(raw_data, size);
    BitStream data(raw_data, size);

    uint8_t count;
    VERIFY(data.read(&count, 8));
    for (uint8_t i = 0; i < count; ++i)
    {
        this->before_string_table_creation();

        std::string name;
        VERIFY(data.read_string(name));
        uint16_t entry_count;
        VERIFY(data.read(&entry_count, 16));

        auto string_table = std::make_shared<StringTable>(std::move(name), entry_count);
        for (uint16_t j = 0; j < entry_count; ++j)
        {
            auto entry = std::make_shared<StringTable::Entry>();
            VERIFY(data.read_string(entry->string));
            string_table->entries.emplace(std::shared_ptr(entry));

            uint8_t has_data;
            VERIFY(data.read(&has_data, 1));
            if (has_data)
            {
                uint16_t data_size;
                VERIFY(data.read(&data_size, 16));

                entry->data.resize(data_size);
                for (uint16_t k = 0; k < data_size; ++k)
                {
                    VERIFY(data.read(&entry->data.at(k), 8));
                }
            }
        }

        this->_string_tables.emplace(std::shared_ptr(string_table));
        this->on_string_table_creation(string_table);
    }

    VERIFY(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

void Client::advance_custom_data(CodedInputStream& stream)
{
    throw GameError("encountered unexpected CUSTOM_DATA event!");
}

bool Client::advance_unknown(CodedInputStream& stream, char command)
{
    throw GameError("encountered unknown command " + std::to_string(command));
}

inline void advance_packet_skip(CodedInputStream& stream)
{
    uint32_t size;
    OK(stream.ReadVarint32(&size));
    OK(stream.Skip(static_cast<int32_t>(size)));
}

void Client::advance_packet_nop(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_disconnect(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_file(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_split_screen_user(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_tick(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_string_command(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_set_console_variable(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_sign_on_state(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_server_info(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

struct UnboundDataTableProperty
{
    // Lifetime strictly shorter than property
    DataTable::DataTableProperty* property;
    std::string name;

    UnboundDataTableProperty(DataTable::DataTableProperty* property, std::string&& name)
        : property(property)
        , name(std::move(name))
    {
    }
};

static void create_data_table_property(
    std::unique_ptr<DataTable::Property>& into,
    std::vector<UnboundDataTableProperty>& collection,
    CSVCMsg_SendTable_sendprop_t&& property_data
)
{
    std::string data_table_name(std::move(*property_data.mutable_dt_name()));
    auto data_table_property = std::make_unique<DataTable::DataTableProperty>(std::move(property_data));
    collection.emplace_back(data_table_property.get(), std::move(data_table_name));
    into = std::move(data_table_property);
}

DatabaseWithName<DataTable> Client::create_data_tables(CodedInputStream& stream)
{
    DatabaseWithName<DataTable> new_data_tables;
    std::vector<UnboundDataTableProperty> new_data_table_properties;

    csgo::message::net::CSVCMsg_SendTable data;
    do
    {
        // This code isn't encapsulated because the final DataTable, which returns true from data.is_end(), is
        // always empty besides that flag; we don't want to bother initializing a bunch of resources for that.
        VERIFY(stream.ExpectTag(csgo::message::net::SVC_Messages::svc_SendTable));
        CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
        VERIFY(limit > 0);

        // Break if we're at the end of the origin.
        data.ParseFromCodedStream(&stream);

        // Actually do event handling if we're not at the terminator.
        if (!data.is_end())
        {
            auto data_table = std::make_shared<DataTable>(data);
            std::unique_ptr<DataTable::DataProperty> preceding_array_element;

            // Do a check to see if our items are all the same type and enumerated; if so we'll turn them into a single
            // array later on.
            bool is_coalesced_array{true};
            DataTable::Property* coalesced_array_element{nullptr};
            size_t coalesced_array_index{0};

            for (CSVCMsg_SendTable_sendprop_t& property_data : *data.mutable_props())
            {
                if (property_data.flags() & DataTable::Property::Flags::EXCLUDE)
                {
                    data_table->excludes.emplace_back(
                        std::move(*property_data.mutable_dt_name()),
                        std::move(*property_data.mutable_var_name())
                    );
                    continue;
                }

                std::unique_ptr<DataTable::Property> property;
                switch (property_data.type())
                {
                    using Kind = DataTable::Property::Kind;
                    case Kind::INT32:
                        property = std::make_unique<DataTable::Int32Property>(std::move(property_data));
                        break;
                    case Kind::FLOAT:
                        property = std::make_unique<DataTable::FloatProperty>(std::move(property_data));
                        break;
                    case Kind::VECTOR3:
                        property = std::make_unique<DataTable::Vector3Property>(std::move(property_data));
                        break;
                    case Kind::VECTOR2:
                        property = std::make_unique<DataTable::Vector2Property>(std::move(property_data));
                        break;
                    case Kind::STRING:
                        property = std::make_unique<DataTable::StringProperty>(std::move(property_data));
                        break;
                    case Kind::ARRAY:
                        VERIFY(preceding_array_element != nullptr);
                        property = std::make_unique<DataTable::ArrayProperty>(
                            std::move(property_data),
                            std::move(preceding_array_element)
                        );
                        break;
                    case Kind::DATA_TABLE:
                        create_data_table_property(
                            property,
                            new_data_table_properties,
                            std::move(property_data)
                        );
                        break;
                    case Kind::INT64:
                        property = std::make_unique<DataTable::Int64Property>(std::move(property_data));
                        break;
                    default:
                        throw csgopp::error::GameError("unreachable");
                }

                // Check if we're still optimizing as array
                if (is_coalesced_array)
                {
                    if (coalesced_array_element == nullptr)
                    {
                        coalesced_array_element = property.get();
                    }

                    if (
                        is_array_index(property->name, coalesced_array_index)
                        && property->equals(coalesced_array_element)
                        )
                    {
                        coalesced_array_index += 1;
                    }
                    else
                    {
                        is_coalesced_array = false;
                    }
                }

                // If there's an array property, the element_type type always precedes it; don't both adding
                if (property->flags & DataTable::Property::Flags::INSIDE_ARRAY)
                {
                    // TODO: maybe there's a safer way to guarantee this?
                    VERIFY(preceding_array_element == nullptr);
                    DataTable::Property* property_pointer = property.release();
                    auto* data_property_pointer = dynamic_cast<DataTable::DataProperty*>(property_pointer);
                    VERIFY(data_property_pointer != nullptr);
                    preceding_array_element = std::unique_ptr<DataTable::DataProperty>(data_property_pointer);
                }
                else
                {
                    data_table->properties.emplace(std::move(property));
                }
            }

            data_table->is_array = data_table->properties.size() > 0 && is_coalesced_array;

            VERIFY(preceding_array_element == nullptr);
            new_data_tables.emplace(std::move(data_table));
        }

        // Related to inline parsing at start of block
        VERIFY(stream.BytesUntilLimit() == 0);
        stream.PopLimit(limit);
    } while (!data.is_end());

    // Link data table properties to their data table
    for (const UnboundDataTableProperty& unbound : new_data_table_properties)
    {
        unbound.property->data_table = lookup(
            new_data_tables.by_name,
            this->_data_tables.by_name,
            unbound.name,
            [&unbound]() { return "failed to find referenced data table " + unbound.name; }
        );
    }

    return new_data_tables;
}

Database<ServerClass> Client::create_server_classes(
    CodedInputStream& stream,
    DatabaseWithName<DataTable>& new_data_tables
)
{
    uint16_t server_class_count;
    VERIFY(demo::read_little_endian_uint16(stream, &server_class_count));
    Database<ServerClass> new_server_classes(server_class_count);

    for (uint16_t i = 0; i < server_class_count; ++i)
    {
        auto server_class = std::make_shared<ServerClass>();
        VERIFY(csgopp::demo::read_little_endian_uint16(stream, &server_class->index));
        VERIFY(csgopp::demo::read_c_style_string(stream, &server_class->name));

        std::string data_table_name;
        VERIFY(csgopp::demo::read_c_style_string(stream, &data_table_name));
        server_class->data_table = lookup(
            new_data_tables.by_name,
            this->_data_tables.by_name,
            data_table_name,
            [&data_table_name]() { return "failed to find referenced data table " + data_table_name; }
        );
        server_class->data_table->server_class = server_class;
        new_server_classes.emplace(std::move(server_class));
    }

    for (const std::shared_ptr<ServerClass>& server_class : new_server_classes)
    {
        for (const std::shared_ptr<DataTable::Property>& property : server_class->data_table->properties)
        {
            if (property->name == "baseclass")
            {
                if (auto data_table_property = std::dynamic_pointer_cast<DataTable::DataTableProperty>(property))
                {
                    ASSERT(server_class->base_class == nullptr, "received two base classes for one server class");
                    VERIFY(data_table_property != nullptr);
                    VERIFY(data_table_property->data_table != nullptr);
                    VERIFY(!data_table_property->data_table->server_class.expired());
                    server_class->base_class = data_table_property->data_table->server_class.lock();
                }
            }
        }
    }

    return new_server_classes;
}

void Client::create_data_tables_and_server_classes(CodedInputStream& stream)
{
    DatabaseWithName<DataTable> new_data_tables = this->create_data_tables(stream);
    Database<ServerClass> new_server_classes = this->create_server_classes(stream, new_data_tables);

    // Materialize types
    for (const std::shared_ptr<ServerClass>& server_class : new_server_classes)
    {
        server_class->data_table->construct_type();
    }

    // Now we can emplace and emit
    this->_data_tables.reserve(new_data_tables.size());
    for (const std::shared_ptr<DataTable>& data_table : new_data_tables)
    {
        this->before_data_table_creation();
        this->_data_tables.emplace(std::shared_ptr(data_table));
        this->on_data_table_creation(data_table);
    }

    this->_server_classes.reserve(new_server_classes.size());
    for (const std::shared_ptr<ServerClass>& server_class : new_server_classes)
    {
        this->before_server_class_creation();
        this->_server_classes.emplace(server_class->index, std::shared_ptr(server_class));
        this->on_server_class_creation(server_class);
    }
}

void Client::advance_packet_send_table(CodedInputStream& stream)
{
    CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
    VERIFY(limit != 0);

    this->create_data_tables_and_server_classes(stream);

    VERIFY(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

void Client::advance_packet_class_info(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_set_pause(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

/// \see https://github.com/markus-wa/demoinfocs-golang/blob/50f55785b7a0ba89164662a000e00cd55969f7ae/pkg/demoinfocs/stringtables.go#L163
void Client::advance_packet_create_string_table(CodedInputStream& stream)
{
    CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
    VERIFY(limit > 0);

    csgo::message::net::CSVCMsg_CreateStringTable data;
    VERIFY(data.ParseFromCodedStream(&stream));

    this->before_string_table_creation();
    auto string_table = std::make_shared<StringTable>(data);
    this->populate_string_table(*string_table, data.string_data(), data.num_entries());
    this->_string_tables.emplace(std::shared_ptr(string_table));
    this->on_string_table_creation(string_table);

    VERIFY(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

void Client::advance_packet_update_string_table(CodedInputStream& stream)
{
    CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
    VERIFY(limit > 0);

    csgo::message::net::CSVCMsg_UpdateStringTable data;
    VERIFY(data.ParseFromCodedStream(&stream));

    size_t index = data.table_id();
    auto string_table = this->_string_tables.at(index);  // TODO revisit if we remove
    ASSERT(string_table != nullptr, "expected a string table at index %zd", index);

    this->before_string_table_update(string_table);
    this->populate_string_table(*string_table, data.string_data(), data.num_changed_entries());
    this->on_string_table_update(string_table);

    VERIFY(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

void Client::populate_string_table(
    StringTable& string_table,
    const std::string& blob,
    int32_t count
)
{
    BitStream string_data(blob);
    uint8_t verification_bit;
    string_data.read(&verification_bit, 1);
    VERIFY(verification_bit == 0);

    Ring<std::string_view, 32> string_table_entry_history;  // 32 appears to be constant

    size_t index_size = csgopp::common::bits::width(string_table.capacity);
    StringTable::Index auto_increment = 0;
    for (int32_t i = 0; i < count; ++i)
    {
        uint8_t use_auto_increment;
        VERIFY(string_data.read(&use_auto_increment, 1));
        if (!use_auto_increment)
        {
            VERIFY(string_data.read(&auto_increment, index_size));
        }

        // Append
        std::shared_ptr<StringTable::Entry> entry;
        if (auto_increment == string_table.entries.size())
        {
            entry = std::make_shared<StringTable::Entry>();
            string_table.entries.emplace(std::shared_ptr(entry));
        }
        else
        {
            entry = string_table.entries.at(auto_increment);
            if (entry == nullptr)
            {
                entry = std::make_shared<StringTable::Entry>();
                string_table.entries.emplace(auto_increment, std::shared_ptr(entry));
            }
        }
        VERIFY(entry != nullptr);

        uint8_t has_string;
        VERIFY(string_data.read(&has_string, 1));
        if (has_string)
        {
            entry->string.clear();

            uint8_t append_to_existing;
            VERIFY(string_data.read(&append_to_existing, 1));
            if (append_to_existing)
            {
                uint8_t history_index;
                VERIFY(string_data.read(&history_index, 5));
                uint8_t bytes_to_copy;
                VERIFY(string_data.read(&bytes_to_copy, 5));
                entry->string.append(string_table_entry_history.at(history_index), 0, bytes_to_copy);
            }

            // Read a c-style string; take until null
            VERIFY(string_data.read_string(entry->string));
        }

        string_table_entry_history.push_back_overwrite(entry->string);
        uint8_t has_data;
        VERIFY(string_data.read(&has_data, 1));
        if (has_data)
        {
            if (string_table.data_fixed)  // < 8 bits
            {
                VERIFY(string_table.data_size_bits <= 8);
                entry->data.push_back(0);
                string_data.read(&entry->data.back(), string_table.data_size_bits);
            }
            else
            {
                uint16_t data_size;
                VERIFY(string_data.read(&data_size, 14));
                entry->data.resize(data_size);
                for (uint16_t j = 0; j < data_size; ++j)
                {
                    VERIFY(string_data.read(&entry->data.at(j), 8));
                }
            }
        }

        if (string_table.name == "userinfo")
        {
            size_t user_index = std::stoull(entry->string);
            this->update_user(user_index, entry->data);
        }

        auto_increment += 1;
    }
}

void Client::advance_packet_voice_initialization(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_voice_data(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_print(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_sounds(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_set_view(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_fix_angle(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_crosshair_angle(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_bsp_decal(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_split_screen(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_user_message(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_entity_message(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_game_event(CodedInputStream& stream)
{
    CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
    VERIFY(limit > 0);

    this->before_game_event();
    csgo::message::net::CSVCMsg_GameEvent data;
    data.ParseFromCodedStream(&stream);

    const std::shared_ptr<GameEventType>& game_event_type = this->_game_event_types.at_id(data.eventid());
    GameEvent game_event(game_event_type);
    game_event.id = data.eventid();

    for (size_t i = 0; i < data.keys_size(); ++i)
    {
        const GameEventType::Member& member = game_event_type->members.at(i);
        csgo::message::net::CSVCMsg_GameEvent_key_t& key = *data.mutable_keys(i);
        auto* game_event_value_type = dynamic_cast<const game_event::DataType*>(member.type.get());
        VERIFY(game_event_value_type != nullptr);
        game_event_value_type->update(game_event.data.get() + member.offset, std::move(key));
    }

    this->on_game_event(std::move(game_event));

    VERIFY(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

/// \sa https://github.com/markus-wa/demoinfocs-golang/blob/1b196ffaaf93c531cdae5897091692e14ead19d2/pkg/demoinfocs/parsing.go#L330
/// \sa https://github.com/markus-wa/demoinfocs-golang/blob/9c61151c71c3821c194f60380cac3777e18e7f6d/pkg/demoinfocs/net_messages.go#L15
void Client::advance_packet_packet_entities(CodedInputStream& stream)
{
    CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
    VERIFY(limit > 0);

    DEBUG(int current_position = stream.CurrentPosition());
    csgo::message::net::CSVCMsg_PacketEntities data;
    VERIFY(data.ParseFromCodedStream(&stream));

    BitStream entity_data(data.entity_data());
    Entity::Id entity_id = 0;
    for (uint32_t i = 0; i < data.updated_entries(); ++i, ++entity_id)
    {
        uint32_t auto_increment_skip;
        entity_data.read_compressed_uint32(&auto_increment_skip);
        entity_id += auto_increment_skip;

        uint8_t command;
        entity_data.read(&command, 2);

        if (command & 0b1)
        {
            if (command & 0b10)
            {
                this->delete_entity(entity_id);
            }
        }
        else
        {
            if (command & 0b10)
            {
                this->create_entity(entity_id, entity_data);
            }
            else
            {
                this->update_entity(entity_id, entity_data);
            }
        }
    }

    VERIFY(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

// THIS MUST BE CALLED BEFORE _update_entity
void Client::_get_update_indices(BitStream& stream)
{
    uint8_t small_increment_optimization;
    OK(stream.read(&small_increment_optimization, 1));

    // Keeping this static GREATLY reduces the number of spurious allocations
    this->_update_entity_indices.clear();

    // It's honestly probably more efficient to read through this twice than it is to allocate and make copies
    uint16_t index = 0;
    while (true)
    {
        uint16_t jump;
        if (small_increment_optimization)
        {
            uint8_t use_auto_index;
            VERIFY(stream.read(&use_auto_index, 1));
            if (use_auto_index)
            {
                jump = 0;
            }
            else
            {
                uint8_t is_small_jump;
                VERIFY(stream.read(&is_small_jump, 1));
                if (is_small_jump)
                {
                    VERIFY(stream.read(&jump, 3));
                }
                else
                {
                    VERIFY(stream.read_compressed_uint16(&jump));
                }
            }
        }
        else
        {
            VERIFY(stream.read_compressed_uint16(&jump));
        }

        if (jump == 0xFFF)
        {
            break;
        }
        else
        {
            index += jump;
        }

        this->_update_entity_indices.emplace_back(index);
        index += 1;
    }
}

// THIS MUST BE CALLED AFTER _get_update_indices
/// \sa https://github.com/markus-wa/demoinfocs-golang/blob/9c61151c71c3821c194f60380cac3777e18e7f6d/pkg/demoinfocs/sendtables/entity.go#L104
void Client::_update_entity(Entity& entity, BitStream& stream)
{
    for (uint16_t i : this->_update_entity_indices)
    {
        // Actually update the field
        const EntityDatum& datum = entity.type->prioritized[i];
        datum.type->update(entity.data.get() + datum.offset, stream, datum.property.get());
    }
}

void Client::create_entity(Entity::Id id, BitStream& stream)
{
    size_t server_class_index_size = csgopp::common::bits::width(this->_server_classes.size()) + 1;
    ServerClass::Index server_class_id;
    VERIFY(stream.read(&server_class_id, server_class_index_size));
    const std::shared_ptr<ServerClass>& server_class = this->_server_classes.at(server_class_id);

    uint16_t serial_number;
    VERIFY(stream.read(&serial_number, ENTITY_HANDLE_SERIAL_NUMBER_BITS));

    this->before_entity_creation(id, server_class);

    VERIFY(server_class->data_table->type() != nullptr);
    std::shared_ptr<Entity> entity = std::make_shared<Entity>(
        server_class->data_table->type(),
        id,
        server_class
    );

    // Update from baseline in string table
    VERIFY(this->_string_tables.instance_baseline != nullptr);
    for (std::shared_ptr<StringTable::Entry>& entry : this->_string_tables.instance_baseline->entries)
    {
        if (std::stoi(entry->string) == server_class->index)
        {
            BitStream baseline(entry->data);
            // TODO: this could be optimized if it's slow--it's probably not
            this->_get_update_indices(baseline);
            this->_update_entity(*entity, baseline);
            break;
        }
    }

    // Update from provided data
    this->_get_update_indices(stream);
    this->_update_entity(*entity, stream);
    this->_entities.emplace(id, std::shared_ptr(entity));
    this->on_entity_creation(entity);
}

/// \sa https://github.com/markus-wa/demoinfocs-golang/blob/9c61151c71c3821c194f60380cac3777e18e7f6d/pkg/demoinfocs/sendtables/entity.go#L104
void Client::update_entity(Entity::Id id, BitStream& stream)
{
    const std::shared_ptr<Entity>& entity = this->_entities.at(id);
    // don't want a callback during entity creation so repeat this code
    this->_get_update_indices(stream);
    this->before_entity_update(entity, this->_update_entity_indices);
    this->_update_entity(*entity, stream);
    this->on_entity_update(entity, this->_update_entity_indices);
}

void Client::delete_entity(Entity::Id id)
{
    VERIFY(id < this->_entities.size());
    std::shared_ptr<Entity> entity = this->_entities.at(id);
    VERIFY(entity != nullptr);

    this->before_entity_deletion(entity);
    this->_entities.at(id) = nullptr;
    this->on_entity_deletion(std::move(entity));
}

void Client::_update_user(User& user, const std::string& data)
{
    ContainerReader<std::string> reader(data);
    user.version = reader.deserialize<uint64_t, BigEndian>();
    user.xuid = reader.deserialize<uint64_t, BigEndian>();
    user.name = reader.terminated(128);
    user.id = reader.deserialize<int32_t, BigEndian>();
    user.guid = reader.terminated(33);
    user.friends_id = reader.deserialize<uint32_t, BigEndian>();
    user.friends_name = reader.terminated(128);
    user.is_fake = reader.deserialize<uint8_t>() != 0;
    user.is_hltv = reader.deserialize<uint8_t>() != 0;
    user.custom_files[0] = reader.deserialize<uint32_t, LittleEndian>();
    user.custom_files[1] = reader.deserialize<uint32_t, LittleEndian>();
    user.custom_files[2] = reader.deserialize<uint32_t, LittleEndian>();
    user.custom_files[3] = reader.deserialize<uint32_t, LittleEndian>();
}

void Client::update_user(size_t index, const std::string& data)
{
    if (data.empty())
    {
        return;
    }

    if (index >= this->_users.size() || this->_users.at(index) == nullptr)
    {
        User::Index user_index = index + 1;
        this->before_user_creation(user_index);
        auto user = std::make_shared<User>(user_index);  // Always entity index + 1
        this->_update_user(*user, data);
        this->_users.emplace(index, std::shared_ptr(user));
        this->on_user_creation(user);
    }
    else
    {
        std::shared_ptr<User>& user = this->_users.at(index);
        this->before_user_update(user);
        this->_update_user(*user, data);
        this->on_user_update(user);
    }
}

void Client::advance_packet_temporary_entities(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_prefetch(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_menu(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_game_event_list(CodedInputStream& stream)
{
    CodedInputStream::Limit limit = stream.ReadLengthAndPushLimit();
    VERIFY(limit > 0);

    csgo::message::net::CSVCMsg_GameEventList data;
    data.ParseFromCodedStream(&stream);
    for (csgo::message::net::CSVCMsg_GameEventList_descriptor_t& descriptor : *data.mutable_descriptors())
    {
        this->before_game_event_type_creation();
        std::shared_ptr<GameEventType> game_event_type = GameEventType::build(std::move(descriptor));
        this->_game_event_types.emplace(std::shared_ptr(game_event_type));  // TODO: ergonomics
        this->on_game_event_type_creation(game_event_type);
    }

    VERIFY(stream.BytesUntilLimit() == 0);
    stream.PopLimit(limit);
}

void Client::advance_packet_get_console_variable_value(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_paintmap_data(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_command_key_values(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_encrypted_data(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_hltv_replay(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_broadcast_command(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_player_avatar_data(CodedInputStream& stream)
{
    advance_packet_skip(stream);
}

void Client::advance_packet_unknown(CodedInputStream& stream, uint32_t command)
{
    throw GameError("unrecognized message " + std::string(demo::describe_net_message(command)));
}

}
