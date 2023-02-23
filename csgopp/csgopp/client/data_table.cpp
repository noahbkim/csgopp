#include "data_table.h"

#include "../common/control.h"
#include "server_class.h"
#include "entity.h"


namespace csgopp::client::data_table
{

using csgopp::error::GameError;
using csgopp::common::object::shared;
using csgopp::common::object::Type;
using csgopp::common::object::shared;
using csgopp::common::control::concatenate;
using csgopp::client::entity::EntityType;

DataTable::DataTable(const CSVCMsg_SendTable& data)
    : name(data.net_table_name())
    , properties(data.props_size())
{
}

std::shared_ptr<const EntityType> DataTable::construct_type()
{
    if (this->_type == nullptr)
    {
        const EntityType* base{nullptr};
        if (this->server_class != nullptr && this->server_class->base_class != nullptr)
        {
            base = this->server_class->base_class->data_table->construct_type().get();
        }

        EntityType::Builder builder(base);
        builder.name = this->name;
        builder.context = this;
        for (DataTable::Property* property: this->properties)
        {
            property->build(builder);
        }

        this->_type = std::make_shared<EntityType>(std::move(builder), this);
    }

    return this->_type;
}

std::shared_ptr<const EntityType> DataTable::type() const
{
    return this->_type;
}

std::shared_ptr<const ArrayType> DataTable::construct_array_type()
{
    if (this->_array_type == nullptr)
    {
        std::shared_ptr<const Type> element_type = this->properties.at(0)->type();

        // We have to manually set offsets since we're not materializing
        for (size_t i = 0; i < this->properties.size(); ++i)
        {
            this->properties.at(i)->offset = i * element_type->size();
        }

        size_t array_size = this->properties.size();
        this->_array_type = std::make_shared<ArrayType>(element_type, array_size);
    }

    return this->_array_type;
}

void DataTable::apply(Cursor<Definition> cursor) const
{
    if (this->server_class != nullptr)
    {
        cursor.target.aliases.emplace_back(this->server_class->name);
    }
}

/// Left pad with zeros to 3 characters
bool is_array_index(std::string_view name, size_t index)
{
    if (name.size() < 3)
    {
        return false;
    }

    size_t i{0};
    for (; i < 3; ++i)
    {
        if (name[name.size() - 1 - i] != '0' + index % 10)
        {
            return false;
        }

        index /= 10;
    }

    for (; i < name.size(); ++i)
    {
        if (index == 0 || name[name.size() - 1 - i] != '0' + index % 10)
        {
            return false;
        }

        index /= 10;
    }

    return true;
}

}
