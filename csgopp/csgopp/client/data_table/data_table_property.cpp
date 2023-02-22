#include "data_table_property.h"
#include "../data_table.h"
#include "../entity.h"

#define GUARD(CONDITION) if (!(CONDITION)) { return false; }
#define CAST(OTHER, TYPE, VALUE) auto* (OTHER) = dynamic_cast<const TYPE*>(VALUE); GUARD((OTHER) != nullptr);
#define EQUAL(OTHER, NAME) ((OTHER)->NAME == this->NAME)

namespace csgopp::client::data_table::data_table_property
{

using csgopp::common::object::ObjectType;

void DataTableProperty::apply(Cursor<Declaration> declaration) const
{
    if (this->data_table->is_array)
    {
        declaration.target.annotations.emplace_back("is array");
    }
}

DataTableProperty::DataTableProperty(CSVCMsg_SendTable_sendprop_t&& data)
    : Property(std::move(data))
{}

Property::Kind::T DataTableProperty::kind() const
{
    return Kind::DATA_TABLE;
}

std::shared_ptr<const Type> DataTableProperty::type() const
{
    if (this->data_table->is_array)
    {
        return this->data_table->construct_array_type();
    }
    else
    {
        return this->data_table->construct_type();
    }
}

void DataTableProperty::build(ObjectType::Builder& builder)
{
    if (this->name == "baseclass")
    {
        // Do nothing; this is already handled
    }
    else if (this->collapsible())
    {
        // TODO: investigate whether this is correct, DT_OverlayVars is only non-baseclass example
        this->offset = builder.embed(this->data_table->construct_type().get());
    }
    else
    {
        Property::build(builder);
    }
}

bool DataTableProperty::equals(const Property* other) const
{
    GUARD(this->flags == other->flags);
    CAST(as, DataTableProperty, other);
    return EQUAL(as, data_table);
}

}
