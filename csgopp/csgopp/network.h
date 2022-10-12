#pragma once

#include <vector>
#include <absl/container/flat_hash_map.h>
#include <google/protobuf/io/coded_stream.h>

#include "network/data_table.h"
#include "network/server_class.h"
#include "network/string_table.h"

namespace csgopp::game
{

// Forward declaration for use in `SimulationObserverBase`.
template<typename Observer>
class Simulation;

}

namespace csgopp::network
{

using csgopp::network::DataTable;
using csgopp::network::ServerClass;
using csgopp::network::StringTable;

/// \brief Low-level, persistent data replicated from the DEMO.
///
/// This struct manages data associated with Valve's server-client
/// replication system. That data includes:
///
///   - Data tables, also referred to as "send tables", which contain
///     nearly all synchronized game data (entities, mainly).
///   - Server classes, which define archetypes for entities and provide
///     reflective information about their properties.
///   - String tables, which serve as a way for the server to relay blobs
///     of data to connected clients (UI strings, entity initialization
///     data, etc.).
///
/// \see `csgopp::network::DataTable`
/// \see `csgopp::network::ServerClass`
/// \see `csgopp::network::StringTable`
class Network
{
public:
    /// Get a container of all current data tables.
    ///
    /// \return A reference to the internal data table vector.
    /// \note Pointers returned by `Network` methods live as long as the
    ///     `Network` and `Simulation` in turn.
    const std::vector<DataTable*>& data_tables() { return this->_data_tables; }

    /// Get a data table by its intrinsic name.
    ///
    /// \param name is the server-defined name of the data table.
    /// \return A raw pointer to the table.
    /// \note Pointers returned by `Network` methods live as long as the
    ///     `Network` and `Simulation` in turn.
    DataTable* data_table(std::string_view name) { return this->_data_tables_by_name.at(name); };

private:
    template<typename Observer>
    friend class csgopp::game::Simulation;

    /// All data tables and properties currently defined by the DEMO.
    ///
    /// Our motivation for encapsulating allocation and management of these
    /// objects in a single struct is essentially that it keeps the calling
    /// code fairly concise while facilitating (eventual) locality
    /// optimization.
    ///
    /// \todo Find an alternative container to std::vector that can
    ///     efficiently handle adding stable pages of elements and provide
    ///     iteration. Maybe Boost's stable_vec.
    ///
    /// @{

    /// Holds all allocated data tables.
    std::vector <std::unique_ptr<DataTable>> _data_table_manager;

    /// Holds all published data tables.
    std::vector<DataTable*> _data_tables;

    /// Holds all allocated data table properties.
    std::vector <std::unique_ptr<DataTable::Property>> _data_table_property_manager;

    /// Data table lookup by server-defined name. No management required.
    absl::flat_hash_map<std::string_view, DataTable*> _data_tables_by_name;

    /// Allocate a new data table and return its stable pointer.
    ///
    /// \tparam Args constructor arguments.
    /// \param args constructor arguments.
    /// \return A pointer to the datatable.
    /// \note The returned pointer has the same lifetime as the `Network`.
    template<typename... Args>
    DataTable* allocate_data_table(Args... args);

    /// Allocate a new data table property T and return its stable pointer.
    ///
    /// \tparam T the `DataTable::Property` subclass to instantiate.
    /// \tparam Args constructor arguments.
    /// \param args constructor arguments.
    /// \return A pointer to the property.
    /// \note The returned pointer has the same lifetime as the `Network`.
    template<typename T, typename... Args>
    DataTable::Property* allocate_data_table_property(Args... args);

    /// Allocate a data table property by type with the given args.
    ///
    /// \tparam Args constructor arguments.
    /// \param type the enumerated type of the property to switch on.
    /// \param args constructor arguments.
    /// \return A pointer to the property.
    /// \note The returned pointer has the same lifetime as the `Network`.
    template<typename... Args>
    DataTable::Property* allocate_data_table_property(
        DataTable::Property::Type::T type,
        Args... args);

    /// Make an allocated data table available for external use.
    void publish_data_table(DataTable* data_table);

    /// @}

    /// All server classes defined by the demo. Same notes as above.
    ///
    /// @{

    /// Holds all allocated server classes.
    std::vector <std::unique_ptr<ServerClass>> _server_class_manager;

    /// Holds all published server classes.
    std::vector<ServerClass*> _server_classes;

    /// A mapping from server-defined name to server class.
    absl::flat_hash_map<std::string_view, ServerClass*> _server_classes_by_name;

    /// A mapping from server-defined ID to server class.
    absl::flat_hash_map<ServerClass::Id, ServerClass*> _server_classes_by_id;

    /// Allocate a new server class.
    ///
    /// \tparam Args constructor arguments.
    /// \param args constructor arguments.
    /// \return A pointer to the server class.
    /// \note The returned pointer has the same lifetime as the `Network`.
    template<typename... Args>
    ServerClass* allocate_server_class(Args... args);

    /// Publish a server class.
    ///
    /// \param server_class a server class to make externally available.
    void publish_server_class(ServerClass* server_class);

    /// @}

    /// All string tables defined by the demo. Same notes as above.
    ///
    /// @{

    /// Holds all allocated string tables.
    std::vector <std::unique_ptr<StringTable>> _string_table_manager;

    /// Holds all published string tables.
    std::vector<StringTable*> _string_tables;

    /// Holds all published string tables.
    std::vector <std::unique_ptr<StringTable::Entry>> _string_table_entry_manager;

    /// A mapping from server-defined name to string table.
    absl::flat_hash_map<std::string_view, StringTable*> _string_tables_by_name;

    /// Allocate a new string table.
    ///
    /// \tparam Args constructor arguments.
    /// \param args constructor arguments.
    /// \return A pointer to the string table.
    /// \note The returned pointer has the same lifetime as the `Network`.
    template<typename... Args>
    StringTable* allocate_string_table(Args... args);

    /// Allocate a new string table entry.
    ///
    /// \tparam Args constructor arguments.
    /// \param args constructor arguments.
    /// \return A pointer to the string table entry.
    /// \note The returned pointer has the same lifetime as the `Network`.
    template<typename... Args>
    StringTable::Entry* allocate_string_table_entry(Args... args);

    /// Publish a string table.
    ///
    /// \param string_table a string table to make externally accessible.
    void publish_string_table(StringTable* string_table);

    /// @}
};

template<typename... Args>
DataTable* Network::allocate_data_table(Args... args)
{
    std::unique_ptr<DataTable> storage = std::make_unique<DataTable>(args...);
    return this->_data_table_manager.emplace_back(std::move(storage)).get();
}

template<typename T, typename... Args>
DataTable::Property* Network::allocate_data_table_property(Args... args)
{
    std::unique_ptr<DataTable::Property> storage = std::make_unique<T>(args...);
    return this->_data_table_property_manager.emplace_back(std::move(storage)).get();
}

template<typename... Args>
DataTable::Property* Network::allocate_data_table_property(
    DataTable::Property::Type::T type,
    Args... args
) {
    switch (type)
    {
        using Type = DataTable::Property::Type;
        case Type::INT32:
            return this->template allocate_data_table_property<DataTable::Int32Property>(args...);
        case Type::FLOAT:
            return this->template allocate_data_table_property<DataTable::FloatProperty>(args...);
        case Type::VECTOR3:
            return this->template allocate_data_table_property<DataTable::Vector3Property>(args...);
        case Type::VECTOR2:
            return this->template allocate_data_table_property<DataTable::Vector2Property>(args...);
        case Type::STRING:
            return this->template allocate_data_table_property<DataTable::StringProperty>(args...);
        case Type::ARRAY:
            return this->template allocate_data_table_property<DataTable::ArrayProperty>(args...);
        case Type::DATA_TABLE:
            return this->template allocate_data_table_property<DataTable::DataTableProperty>(args...);
        case Type::INT64:
            return this->template allocate_data_table_property<DataTable::Int64Property>(args...);
        default:
            throw csgopp::error::GameError("unreachable");
    }
}

template<typename... Args>
ServerClass* Network::allocate_server_class(Args... args)
{
    std::unique_ptr<ServerClass> storage = std::make_unique<ServerClass>(args...);
    return this->_server_class_manager.emplace_back(std::move(storage)).get();
}

template<typename... Args>
StringTable* Network::allocate_string_table(Args... args)
{
    std::unique_ptr<StringTable> storage = std::make_unique<StringTable>(args...);
    return this->_string_table_manager.emplace_back(std::move(storage)).get();
}

template<typename... Args>
StringTable::Entry* Network::allocate_string_table_entry(Args... args)
{
    std::unique_ptr<StringTable::Entry> storage = std::make_unique<StringTable::Entry>(args...);
    return this->_string_table_entry_manager.emplace_back(std::move(storage)).get();
}

}
