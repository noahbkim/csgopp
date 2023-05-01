#pragma once

#include <variant>
#include <absl/container/flat_hash_map.h>
#include <google/protobuf/io/coded_stream.h>

#include "../common/database.h"
#include "../common/lookup.h"
#include "../common/macro.h"
#include "../common/vector.h"
#include "../error.h"
#include "data_table/data_property.h"
#include "data_table/data_table_property.h"
#include "netmessages.pb.h"
#include <objective/code.h>
#include <objective.h>

namespace csgopp::client::server_class
{

struct ServerClass;

}

namespace csgopp::client::entity
{

struct EntityType;

}

/// This namespace defines a framework for efficiently representing sendtables
/// in memory. It is closely integrated with the `common::objective` library,
/// which we use to build and use entity types at run time.
namespace csgopp::client::data_table
{

using csgo::message::net::CSVCMsg_SendTable;
using csgopp::client::entity::EntityType;
using csgopp::client::server_class::ServerClass;
using csgopp::common::database::DatabaseWithName;
using csgopp::error::GameError;
using objective::ArrayType;
using objective::code::Metadata;
using objective::code::Declaration;

using Exclude = std::pair<std::string, std::string>;

/// \brief A collection of properties that mirror a send/receive table.
///
/// Data tables are a core part of the source engine's data low-latency data
/// synchronization framework. Specifically, data tables flexibly and
/// efficiently format C-like data structures; they are used to statically
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
///     Since different properties have fairly unique behavior, we format
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
struct DataTable : Metadata<Declaration&>
{
    // Expose these for convenience
    using Property = property::Property;
    using DataProperty = data_property::DataProperty;
    using Int32Property = data_property::Int32Property;
    using FloatProperty = data_property::FloatProperty;
    using Vector3Property = data_property::Vector3Property;
    using Vector2Property = data_property::Vector2Property;
    using StringProperty = data_property::StringProperty;
    using ArrayProperty = data_property::ArrayProperty;
    using Int64Property = data_property::Int64Property;
    using DataTableProperty = data_table_property::DataTableProperty;

    // Internal use
    using PropertyDatabase = DatabaseWithName<Property>;

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
    // TODO: std::shared_ptr<const DataTable> base_class;
    std::weak_ptr<ServerClass> server_class;

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

    /// \brief Construct the underlying EntityType of this data table.
    std::shared_ptr<const EntityType> construct_type();

    /// \brief Return the constructed type; not possible to guarantee this statically.
    ///
    /// \return a shared pointer to the allocated type.
    [[nodiscard]] std::shared_ptr<const EntityType> type() const;

    /// \brief Build an array entity type from the data table properties.
    ///
    /// \return a shared pointer to the allocated type.
    std::shared_ptr<const ArrayType> construct_array_type();

    /// \brief Annotate a struct definition generated by the entity type.
    ///
    /// \param cursor the code generation cursor to modify.
    void attach(Declaration& declaration) override;

private:
    std::shared_ptr<const EntityType> _type;
    std::shared_ptr<const ArrayType> _array_type;
};

using DataTableDatabase = DatabaseWithName<DataTable>;

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
    DEFAULT(throw GameError("unknown send table property type: " + std::to_string(key)))
)

}
