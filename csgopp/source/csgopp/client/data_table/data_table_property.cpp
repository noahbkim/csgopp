#include "csgopp/client/data_table/data_table_property.h"
#include "csgopp/client/data_table.h"
#include "csgopp/client/entity.h"

#define GUARD(CONDITION) if (!(CONDITION)) { return false; }
#define CAST(OTHER, TYPE, VALUE) auto* (OTHER) = dynamic_cast<const TYPE*>(VALUE); GUARD((OTHER) != nullptr);
#define EQUAL(OTHER, NAME) ((OTHER)->NAME == this->NAME)

namespace csgopp::client::data_table::data_table_property
{

using objective::ObjectType;

void DataTableProperty::attach(Declaration& declaration, Declaration::Member& member)
{
    if (this->data_table->is_array)
    {
        member.annotations.emplace_back("is array");
    }
}

DataTableProperty::DataTableProperty(CSVCMsg_SendTable_sendprop_t&& data)
    : Property(std::move(data))
{
}

DataTableProperty::DataTableProperty(CSVCMsg_SendTable_sendprop_t&& data, std::shared_ptr<DataTable> data_table)
    : Property(std::move(data))
    , data_table(std::move(data_table))
{
}

Property::Kind::T DataTableProperty::kind() const
{
    return Kind::DATA_TABLE;
}

std::shared_ptr<const Type> DataTableProperty::construct_type()
{
    if (this->type() == nullptr)
    {
        assert(this->data_table != nullptr);
        if (this->data_table->is_array)
        {
            this->_type = this->data_table->construct_array_type();
        }
        else
        {
            this->_type = this->data_table->construct_type();
        }
    }
    return this->_type;
}

std::shared_ptr<const Type> DataTableProperty::type() const
{
    return this->_type;
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
        this->offset = builder.embed(*this->data_table->construct_type());
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
