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
using csgopp::client::entity::PropertyType;
using csgopp::client::entity::ArrayType;
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
    struct Type
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
    [[nodiscard]] virtual Type::T type() const = 0;

    /// \brief Materialize a `PropertyType` from the property.
    ///
    /// \return a `std::shared_ptr` to a `PropertyType` that
    ///     corresponds to this object.
    ///
    /// This method is used internally to construct the dynamic type associated
    /// with each server class. It's important that we adhere to
    /// `csgopp::common::object`'s use of `std::shared_ptr` because there isn't
    /// always a clear owner for a given type.
    ///
    /// \sa `csgopp::client::entity`
    /// \sa `csgopp::common::object`
    [[nodiscard]] virtual std::shared_ptr<const PropertyType> materialize() const = 0;

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
    [[nodiscard]] constexpr bool collapsible() const;

    [[nodiscard]] constexpr bool changes_often() const;
};

/// \brief Represents an integer with maximum width 32 bits. Can be boolean.
struct Int32Property : public Property
{
    int32_t bits;

    explicit Int32Property(CSVCMsg_SendTable_sendprop_t&& data);
    [[nodiscard]] Type::T type() const override;
    [[nodiscard]] std::shared_ptr<const PropertyType> materialize() const override;
    [[nodiscard]] bool equals(const Property* other) const override;
};

/// \brief Represents a float.
struct FloatProperty : public Property
{
    float high_value;
    float low_value;
    int32_t bits;

    explicit FloatProperty(CSVCMsg_SendTable_sendprop_t&& data);
    [[nodiscard]] Type::T type() const override;
    [[nodiscard]] std::shared_ptr<const PropertyType> materialize() const override;
    [[nodiscard]] bool equals(const Property* other) const override;
};

/// \brief Represents a 3D floating point vector.
struct Vector3Property : public Property
{
    using Property::Property;
    [[nodiscard]] Type::T type() const override;
    [[nodiscard]] std::shared_ptr<const PropertyType> materialize() const override;
    [[nodiscard]] bool equals(const Property* other) const override;
};

/// \brief Represents a 2D floating point vector.
struct Vector2Property : public Property
{
    using Property::Property;
    [[nodiscard]] Type::T type() const override;
    [[nodiscard]] std::shared_ptr<const PropertyType> materialize() const override;
    [[nodiscard]] bool equals(const Property* other) const override;
};

/// \brief Represents a string of arbitrary length.
struct StringProperty : public Property
{
    using Property::Property;
    [[nodiscard]] Type::T type() const override;
    [[nodiscard]] std::shared_ptr<const PropertyType> materialize() const override;
    [[nodiscard]] bool equals(const Property* other) const override;
};

/// \brief Represents a fixed-length array of a single other property type.
///
/// Array properties manage their own element type. It seems like arrays are
/// mostly for short arrays (pairs, coordinates, etc.).
struct ArrayProperty : public Property
{
    std::unique_ptr<Property> element;
    int32_t size;

    explicit ArrayProperty(CSVCMsg_SendTable_sendprop_t&& data, Property* element);
    [[nodiscard]] Type::T type() const override;
    [[nodiscard]] std::shared_ptr<const PropertyType> materialize() const override;
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
struct DataTableProperty : public Property
{
    DataTable* data_table{nullptr};

    // No constructor because data_table is set later on
    explicit DataTableProperty(CSVCMsg_SendTable_sendprop_t&& data);
    [[nodiscard]] Type::T type() const override;
    [[nodiscard]] std::shared_ptr<const PropertyType> materialize() const override;

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
};

struct Int64Property : public Property
{
    int32_t bits;

    explicit Int64Property(CSVCMsg_SendTable_sendprop_t&& data);
    [[nodiscard]] Type::T type() const override;
    [[nodiscard]] std::shared_ptr<const PropertyType> materialize() const override;
    [[nodiscard]] bool equals(const Property* other) const override;
};

struct DataTable : Context<Definition>
{
    using Property = Property;
    using Int32Property = Int32Property;
    using FloatProperty = FloatProperty;
    using Vector3Property = Vector3Property;
    using Vector2Property = Vector2Property;
    using StringProperty = StringProperty;
    using ArrayProperty = ArrayProperty;
    using DataTableProperty = DataTableProperty;
    using Int64Property = Int64Property;

    using PropertyDatabase = DatabaseWithName<Property, Delete<Property>>;
    using Exclude = std::pair<std::string, std::string>;
    using ExcludeView = std::pair<std::string_view, std::string_view>;

    std::string name;
    PropertyDatabase properties;  // pointer stability is very convenient
    ServerClass* server_class{nullptr};
    std::shared_ptr<const EntityType> entity_type;
    std::vector<Exclude> excludes;
    bool is_array{false};  // Whether we can make array when member

    DataTable() = default;
    explicit DataTable(const CSVCMsg_SendTable& data);

    std::shared_ptr<const EntityType> materialize();
    std::shared_ptr<const ArrayType> materialize_array();
    void apply(Cursor<Definition> cursor) const override;
};

using DataTableDatabase = DatabaseWithName<DataTable, Delete<DataTable>>;

LOOKUP(describe, DataTable::Property::Type::T, const char*,
    CASE(DataTable::Property::Type::INT32, "INT32")
    CASE(DataTable::Property::Type::FLOAT, "FLOAT")
    CASE(DataTable::Property::Type::VECTOR3, "VECTOR3")
    CASE(DataTable::Property::Type::VECTOR2, "VECTOR2")
    CASE(DataTable::Property::Type::STRING, "STRING")
    CASE(DataTable::Property::Type::ARRAY, "ARRAY")
    CASE(DataTable::Property::Type::DATA_TABLE, "DATA_TABLE")
    CASE(DataTable::Property::Type::INT64, "INT64")
    DEFAULT(throw GameError("unknown send table property type: " + std::to_string(key))));

// https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/dt_send.cpp
bool is_array_index(std::string_view name, size_t index);

}
