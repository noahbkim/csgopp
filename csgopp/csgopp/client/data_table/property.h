#pragma once

#include "netmessages.pb.h"
#include "../../common/bits.h"
#include "../../common/code.h"
#include "../../common/object.h"

namespace csgopp::client::data_table::property
{

using csgo::message::net::CSVCMsg_SendTable_sendprop_t;
using csgopp::common::bits::BitStream;
using csgopp::common::code::Context;
using csgopp::common::code::Declaration;
using csgopp::common::object::ObjectType;
using csgopp::common::object::Type;

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

    /// \brief An absolute object offset.
    ///
    /// This object serves as a lens into objects initialized via the parent
    /// `DataTable::entity_type`. This we have to keep track of this here
    /// because the entity type and prioritized property list (which we use to
    /// update the entity) passes occur separately and assume different
    /// invariants.
    size_t offset;

    /// \brief Construct a property from serialized sendtable data.
    ///
    /// \param data a protobuf container of sendtable property data.
    explicit Property(CSVCMsg_SendTable_sendprop_t&& data)
        : name(std::move(*data.mutable_var_name()))
        , flags(data.flags())
        , priority(data.priority())
        , offset(0)
    {
    }

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
    [[nodiscard]] virtual std::shared_ptr<const Type> type() const = 0;

    /// \brief Attach this property to an `EntityType`
    ///
    /// \param builder an `EntityType::Builder` to attach this member to.
    ///
    /// This has been extracted from the `DataTable::materialize()` logic
    /// because there are special cases regarding member construction,
    /// specifically with data table properties.
    ///
    /// \sa `DataTableProperty::build`
    virtual void build(ObjectType::Builder& builder)
    {
        this->offset = builder.member(this->name, this->type(), this);
    }

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
    [[nodiscard]] virtual bool equals(const Property* other) const
    {
        return false;
    }

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

}