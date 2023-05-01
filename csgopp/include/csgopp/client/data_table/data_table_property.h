#pragma once

#include "netmessages.pb.h"
#include "property.h"
#include <objective/code.h>
#include <objective.h>

namespace csgopp::client::data_table
{

struct DataTable;

}

namespace csgopp::client::data_table::data_table_property
{

using csgo::message::net::CSVCMsg_SendTable_sendprop_t;
using csgopp::client::data_table::property::Property;
using objective::code::Declaration;
using objective::ObjectType;
using objective::Type;

/// \brief Represents the objective described by another data table.
///
/// `DataTable` properties are used to format nested structs in the entity
/// data structure. Most of the time, data tables correspond to allocated
/// `ServerClass` instances, but they can also contain arbitrary nested data
/// as well as format separately serialized arrays. These cases have to be
/// accounted for in `DataTableProperty::build()`.
///
/// \sa https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/dt_send.cpp#L691
struct DataTableProperty final : public Property
{
    std::shared_ptr<DataTable> data_table;

    explicit DataTableProperty(CSVCMsg_SendTable_sendprop_t&& data);
    DataTableProperty(CSVCMsg_SendTable_sendprop_t&& data, std::shared_ptr<DataTable> data_table);

    [[nodiscard]] Kind::T kind() const override;

    [[nodiscard]] std::shared_ptr<const Type> construct_type() override;
    [[nodiscard]] std::shared_ptr<const Type> type() const;

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
    void build(ObjectType::Builder& builder) override;

    [[nodiscard]] bool equals(const Property* other) const override;

    void attach(Declaration& declaration, Declaration::Member& member) override;

private:
    std::shared_ptr<const Type> _type;
};

}