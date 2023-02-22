#pragma once

#include <variant>
#include <absl/container/flat_hash_map.h>
#include <google/protobuf/io/coded_stream.h>

#include "netmessages.pb.h"
#include "../error.h"
#include "../common/lookup.h"
#include "../common/database.h"
#include "../common/vector.h"
#include "../common/macro.h"
#include "../common/code.h"
#include "entity.h"

namespace csgopp::client::server_class
{

struct ServerClass;

}

/// This namespace defines a framework for efficiently representing sendtables
/// in memory. It is closely integrated with the `common::object` library,
/// which we use to build and use entity types at run time.
namespace csgopp::client::data_table
{

using google::protobuf::io::CodedInputStream;
using csgo::message::net::CSVCMsg_SendTable;
using csgo::message::net::CSVCMsg_SendTable_sendprop_t;
using csgopp::error::GameError;
using csgopp::common::bits::BitStream;
using csgopp::common::vector::Vector3;
using csgopp::common::vector::Vector2;
using csgopp::common::database::DatabaseWithName;
using csgopp::common::database::Delete;
using csgopp::common::database::NameTableMixin;
using csgopp::common::code::Context;
using csgopp::common::code::Definition;
using csgopp::common::code::Dependencies;
using csgopp::common::code::Declaration;
using csgopp::common::code::Cursor;
using csgopp::client::server_class::ServerClass;
using csgopp::client::entity::PropertyArrayType;
using csgopp::client::entity::EntityType;
using csgopp::client::entity::Offset;

/// \brief Flags attached to each property by whoever defines it.
///
/// Flags are used everywhere from property and server class creation to data
/// serialization and deserialization.
///
/// \sa https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/dt_common.h
struct PropertyFlags
{
    using T = int32_t;
    enum : T
    {
        UNSIGNED = 1 << 0,
        COORDINATES = 1 << 1,
        NO_SCALE = 1 << 2,
        ROUND_DOWN = 1 << 3,
        ROUND_UP = 1 << 4,
        NORMAL = 1 << 5,
        EXCLUDE = 1 << 6,
        XYZ = 1 << 7,
        INSIDE_ARRAY = 1 << 8,
        PROXY_ALWAYS_YES = 1 << 9,
        IS_VECTOR_ELEMENT = 1 << 10,
        COLLAPSIBLE = 1 << 11,
        COORDINATES_MULTIPLAYER = 1 << 12,
        COORDINATES_MULTIPLAYER_LOW_PRECISION = 1 << 13,
        COORDINATES_MULTIPLAYER_INTEGRAL = 1 << 14,
        CELL_COORDINATES = 1 << 15,
        CELL_COORDINATES_LOW_PRECISION = 1 << 16,
        CELL_COORDINATES_INTEGRAL = 1 << 17,
        CHANGES_OFTEN = 1 << 18,
        VARIABLE_INTEGER = 1 << 19,

        // Computed
        FLOAT_FLAGS = COORDINATES
            | NORMAL
            | COORDINATES_MULTIPLAYER
            | COORDINATES_MULTIPLAYER_LOW_PRECISION
            | COORDINATES_MULTIPLAYER_INTEGRAL
            | CELL_COORDINATES
            | CELL_COORDINATES_LOW_PRECISION
            | CELL_COORDINATES_INTEGRAL,

        HIGH_PRIORITY_FLOAT_FLAGS = COORDINATES
            | COORDINATES_MULTIPLAYER
            | COORDINATES_MULTIPLAYER_LOW_PRECISION
            | COORDINATES_MULTIPLAYER_INTEGRAL,
    };
};

/// \brief A property of a data table.
///
/// A data table property can declare one of the following things:
///
///   - Most commonly, a data table property corresponds to a primitive type
///     that we want to synchronize over the network. In this case, the
///     property will mostly just contains serialization details.
///   - A data table property can also be a pointer to another data table.
///     This just means instead of a primitive, the type of the corresponding
///     member is another data class.
///   - An array property contains a nested element type and length. The
///     element is another property, i.e. something on this list.
///
/// Because there are a variety of options, it's convenient to have a uniform,
/// virtual API so that each kind of property can define its own behavior. This
/// class defines that interface.
///
/// \note It's easiest to declare this outside of `DataTable` so we can forward
///     declare it in other files/not clutter the `DataTable` definition.
///
/// \sa https://developer.valvesoftware.com/wiki/Networking_Entities#Network_Data_Tables
struct Property : Context<Declaration>
{
    /// \brief The type of property as reported by the serialized data.
    /// \sa https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/dt_common.h
    struct Kind
    {
        using T = int32_t;
        enum E : T
        {
            INT32 = 0,
            FLOAT = 1,
            VECTOR3 = 2,
            VECTOR2 = 3,
            STRING = 4,
            ARRAY = 5,
            DATA_TABLE = 6,
            INT64 = 7,
        };
    };

    using Flags = PropertyFlags;
    using Priority = int32_t;

    /// The name of the property.
    std::string name;

    /// Flags associated with the property. Transcends individual types.
    Flags::T flags;

    /// \brief The priority of the property.
    ///
    /// In order to reduce packet size, properties are indexed by priority so
    /// that iterating/jumping to each changed property while updating entities
    /// requires as few bytes as possible. See `DataTable::materialize()`.
    Priority priority;

    /// \brief An absolute object offset and data type.
    ///
    /// This object serves as a lens into objects initialized via the parent
    /// `DataTable::entity_type`. This we have to keep track of this here
    /// because the entity type and prioritized property list (which we use to
    /// update the entity) passes occur separately and assume different
    /// invariants.
    Offset offset;

    /// \brief Construct a property from serialized sendtable data.
    ///
    /// \param data a protobuf container of sendtable property data.
    explicit Property(CSVCMsg_SendTable_sendprop_t&& data);

    /// \brief Materialize the type enumeration of the property.
    ///
    /// \return The network property type.
    ///
    /// It would be redundant to store this when it's discriminated by the
    /// subclass type, but we still want to be able to access it. If you find
    /// yourself switching on this type, you might want to reconsider why
    /// you're not working directly with the virtual interface.
    [[nodiscard]] virtual Kind::T kind() const = 0;

    /// \brief Materialize a `object::Type` from the property.
    ///
    /// \return a `std::shared_ptr` to a `object::Type` that
    ///     corresponds to this object.
    ///
    /// This method is used internally to construct the dynamic type associated
    /// with each server class. It's important that we adhere to
    /// `csgopp::common::object`'s use of `std::shared_ptr` because there isn't
    /// always a clear owner for a given type.
    ///
    /// \sa `csgopp::client::entity`
    /// \sa `csgopp::common::object`
    [[nodiscard]] virtual std::shared_ptr<const common::object::Type> materialize() const = 0;

    /// \brief Attach this property to an `EntityType`
    ///
    /// \param builder an `EntityType::Builder` to attach this member to.
    ///
    /// This has been extracted from the `DataTable::materialize()` logic
    /// because there are special cases regarding member construction,
    /// specifically with data table properties.
    ///
    /// \sa `DataTableProperty::build`
    virtual void build(EntityType::Builder& builder);

    /// \brief Annotate declarations generated from this property's type.
    ///
    /// \param declaration the declaration we want to modify.
    ///
    /// This override is currently just a proof of concept that's helped iron
    /// out the rather circular relationship between the `DataTable` structures
    /// and the `Type` ones.
    // void apply(Cursor<Declaration> declaration) const override;

    /// \brief Check if two properties have the same type.
    ///
    /// \param other another deserialized property.
    /// \return whether they are interchangeable besides the name.
    ///
    /// This method is part of an optimization regarding array serialization
    /// in data tables. It's explained `Client::create_data_tables`.
    [[nodiscard]] virtual bool equals(const Property* other) const;

    /// \brief Check if the property is collapsible.
    ///
    /// \return Whether the COLLAPSIBLE flag is set.
    ///
    /// This should only be set on data table properties; it signifies that the
    /// data table's fields should be directly embedded into the parent type.
    ///
    /// \sa `DataTableProperty::build`
    /// \sa https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/dt_common.h#L74
    [[nodiscard]] inline bool collapsible() const
    {
        return this->flags & Flags::COLLAPSIBLE;
    }

    /// \brief Check if the property changes often.
    ///
    /// \return Whether the CHANGES_OFTEN flag is set.
    ///
    /// This flag factors into how the offsets are prioritized; properties
    /// marked as CHANGES_OFTEN are given priority 64.
    ///
    /// \sa `DataTable::materialize`
    [[nodiscard]] inline bool changes_often() const
    {
        return this->flags & Flags::CHANGES_OFTEN;
    }
};

/// \brief Represents an integer with maximum width 32 bits. Can be boolean.
struct Int32Property final : public Property
{
    int32_t bits;

    explicit Int32Property(CSVCMsg_SendTable_sendprop_t&& data);
    [[nodiscard]] Kind::T kind() const override;
    [[nodiscard]] std::shared_ptr<const common::object::Type> materialize() const override;
    [[nodiscard]] bool equals(const Property* other) const override;
};

/// \brief Represents a float.
struct FloatProperty final : public Property
{
    float high_value;
    float low_value;
    int32_t bits;

    explicit FloatProperty(CSVCMsg_SendTable_sendprop_t&& data);
    [[nodiscard]] Kind::T kind() const override;
    [[nodiscard]] std::shared_ptr<const common::object::Type> materialize() const override;
    [[nodiscard]] bool equals(const Property* other) const override;
};

/// \brief Represents a 3D floating point vector.
struct Vector3Property final : public Property
{
    float high_value;
    float low_value;
    int32_t bits;

    explicit Vector3Property(CSVCMsg_SendTable_sendprop_t&& data);
    [[nodiscard]] Kind::T kind() const override;
    [[nodiscard]] std::shared_ptr<const common::object::Type> materialize() const override;
    [[nodiscard]] bool equals(const Property* other) const override;
};

/// \brief Represents a 2D floating point vector.
struct Vector2Property final : public Property
{
    float high_value;
    float low_value;
    int32_t bits;

    explicit Vector2Property(CSVCMsg_SendTable_sendprop_t&& data);
    [[nodiscard]] Kind::T kind() const override;
    [[nodiscard]] std::shared_ptr<const common::object::Type> materialize() const override;
    [[nodiscard]] bool equals(const Property* other) const override;
};

/// \brief Represents a string of arbitrary length.
struct StringProperty final : public Property
{
    using Property::Property;
    [[nodiscard]] Kind::T kind() const override;
    [[nodiscard]] std::shared_ptr<const common::object::Type> materialize() const override;
    [[nodiscard]] bool equals(const Property* other) const override;
};

/// \brief Represents a fixed-length array of a single other property type.
///
/// Array properties manage their own element type. It seems like arrays are
/// mostly for short arrays (pairs, coordinates, etc.).
struct ArrayProperty final : public Property
{
    std::unique_ptr<Property> element;
    int32_t length;

    ArrayProperty(CSVCMsg_SendTable_sendprop_t&& data, Property* element);
    [[nodiscard]] Kind::T kind() const override;
    [[nodiscard]] std::shared_ptr<const common::object::Type> materialize() const override;
    [[nodiscard]] bool equals(const Property* other) const override;
};

/// \brief Represents the object described by another data table.
///
/// `DataTable` properties are used to represent nested structs in the entity
/// data structure. Most of the time, data tables correspond to allocated
/// `ServerClass` instances, but they can also contain arbitrary nested data
/// as well as represent separately serialized arrays. These cases have to be
/// accounted for in `DataTableProperty::build()`.
///
/// \sa https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/dt_send.cpp#L691
struct DataTableProperty final : public Property
{
    DataTable* data_table{nullptr};

    // No constructor because data_table is set later on
    explicit DataTableProperty(CSVCMsg_SendTable_sendprop_t&& data);
    [[nodiscard]] Kind::T kind() const override;
    [[nodiscard]] std::shared_ptr<const common::object::Type> materialize() const override;

    /// \brief Add the referenced data table as a member of the `EntityType`.
    ///
    /// \param builder the builder to add our member(s) to.
    ///
    /// There are a couple of discrete behaviors covered by this method:
    ///
    ///   - If this data table property represents the containing data table's
    ///     base class (i.e. `this->name == "baseclass"`), don't do anything,
    ///     as our materialized `EntityType` should already be set as the
    ///     builder's `EntityType::Builder::base`.
    ///   - If the referenced data table is collapsible, we directly embed its
    ///     members as our own.
    ///   - Otherwise, just add as a member normally.
    void build(EntityType::Builder& builder) override;

    [[nodiscard]] bool equals(const Property* other) const override;

    void apply(Cursor<Declaration> declaration) const override;
};

/// \brief Represents an integer with maximum width 64 bits.
struct Int64Property final : public Property
{
    int32_t bits;

    explicit Int64Property(CSVCMsg_SendTable_sendprop_t&& data);
    [[nodiscard]] Kind::T kind() const override;
    [[nodiscard]] std::shared_ptr<const common::object::Type> materialize() const override;
    [[nodiscard]] bool equals(const Property* other) const override;
};

// TODO: DELETe
using Exclude = std::pair<std::string, std::string>;
using ExcludeView = std::pair<std::string_view, std::string_view>;

/// \brief A collection of properties that mirror a send/receive table.
///
/// Data tables are a core part of the source engine's data low-latency data
/// synchronization framework. Specifically, data tables flexibly and
/// efficiently represent C-like data structures; they are used to statically
/// specify the properties of `Entity` objects used over the course of a game
/// or demo.
///
/// According to source, data tables have three parts:
///
///   - The name of a data table usually relates to the corresponding class or
///     struct in the game source, but can be arbitrary.
///   - Data table properties describe a member of the represented structure:
///     each property has its own name, a type (int, string, array of
///     properties, etc.) and some metadata regarding values and serialization.
///     Since different properties have fairly unique behavior, we represent
///     them with the different subclasses of the abstract `Property` class.
///   - Excludes mark data table properties that are unused by a given server
///     class/entity. Entities may wish to nested properties, so excludes are
///     represented as a (data table name, property name) pair.
///
/// All `ServerClass`'s, which can be thought of as the "type" of an entity,
/// have a corresponding data table. However, not all data tables are directly
/// pointed to by a server class. Some data tables are nested within others
/// (like nested structs in C/C++), while others are a special case of array
/// serialization (when you see members 000, 001, etc.). These special cases
/// are discussed more in `DataTableProperty`.
///
/// Like structs in C/C++, data tables can inherit from each other. The parent
/// "class" data table is serialized as a data table property with the name
/// `"baseclass"`. While we hold a separate reference to the base class data
/// table, we still add it as a property so that we can correctly construct
/// `Entity::prioritized`.
///
/// \note The nomenclature is a little bit unclear, but "send tables" and
///     "receive tables" refer to the same concept; they simply differentiate
///     between whether you're serializing or deserializing. While we're only
///     receiving data, this seemed like the clearest name.
///
/// \sa https://developer.valvesoftware.com/wiki/Networking_Entities#Network_Data_Tables
/// \sa https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/dt_send.h
/// \sa https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/dt_send.cpp
/// \sa https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/dt_recv.h
/// \sa https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/dt_recv.cpp
struct DataTable : Context<Definition>
{
    // Expose these for convenience
    using Property = Property;
    using Int32Property = Int32Property;
    using FloatProperty = FloatProperty;
    using Vector3Property = Vector3Property;
    using Vector2Property = Vector2Property;
    using StringProperty = StringProperty;
    using ArrayProperty = ArrayProperty;
    using DataTableProperty = DataTableProperty;
    using Int64Property = Int64Property;

    // Internal use
    using PropertyDatabase = DatabaseWithName<Property, Delete<Property>>;

    /// \brief The name of the data table.
    std::string name;

    /// \brief An ordered container of database properties.
    ///
    /// We have to allocate properties to take advantage of the abstract
    /// `Property` interface, but it's also useful because we want the pointers
    /// to be usable in other contexts, e.g. `Offset` objects.
    PropertyDatabase properties;

    /// \brief A collection of excluded data table properties.
    ///
    /// All excludes in a given data table must be coalesced during traversal
    /// anyway, so there's no point in making this a hash set now.
    std::vector<Exclude> excludes;

    // Additional, computed members

    /// \brief The server class defined by this data table.
    ServerClass* server_class{nullptr};

    /// \brief A dynamic type system type for the corresponding structure.
    ///
    /// The `csgopp::common::object` namespace provides way to construct and
    /// instantiate structs at runtime. `EntityType` is a subclass of `Object`
    /// that provides some additional, data-table-related functionality.
    std::shared_ptr<const EntityType> entity_type;

    /// \brief Whether this data table can be represented by an array.
    ///
    /// This condition is always true when the data table has been serialized
    /// as a member array. Unfortunately, there is no discrete flag that marks
    /// such data tables; it is possible (though unlikely) that data tables
    /// that are not intended to be treated as arrays could set this flag.
    ///
    /// This condition is set in `Client::create_data_tables`.
    ///
    /// \sa https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/dt_send.cpp
    bool is_array{false};

    DataTable() = default;

    /// \brief Instantiate a new data table from serialized send table data.
    explicit DataTable(const CSVCMsg_SendTable& data);

    /// \brief Build and entity type from the data table properties.
    ///
    /// \return a shared pointer to the allocated type.
    std::shared_ptr<const EntityType> materialize();

    /// \brief Build an array entity type from the data table properties.
    ///
    /// \return a shared pointer to the allocated type.
    std::shared_ptr<const PropertyArrayType> materialize_array();

    /// \brief Annotate a struct definition generated by the entity type.
    ///
    /// \param cursor the code generation cursor to modify.
    void apply(Cursor<Definition> cursor) const override;
};

using DataTableDatabase = DatabaseWithName<DataTable, Delete<DataTable>>;

/// \brief Determine whether a data table property name is an array index.
///
/// \param name the name of the data table property.
/// \param index the index of the property.
/// \return whether the string corresponds to the given index.
///
/// Array indices are simply a string index left-padded to three digits with
/// zeros. The source only defines 1024 possible indices but whatever.
///
/// \sa https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/dt_send.cpp
bool is_array_index(std::string_view name, size_t index);

LOOKUP(describe, DataTable::Property::Kind::T, const char*,
    CASE(DataTable::Property::Kind::INT32, "INT32")
    CASE(DataTable::Property::Kind::FLOAT, "FLOAT")
    CASE(DataTable::Property::Kind::VECTOR3, "VECTOR3")
    CASE(DataTable::Property::Kind::VECTOR2, "VECTOR2")
    CASE(DataTable::Property::Kind::STRING, "STRING")
    CASE(DataTable::Property::Kind::ARRAY, "ARRAY")
    CASE(DataTable::Property::Kind::DATA_TABLE, "DATA_TABLE")
    CASE(DataTable::Property::Kind::INT64, "INT64")
    DEFAULT(throw GameError("unknown send table property type: " + std::to_string(key))));

}
