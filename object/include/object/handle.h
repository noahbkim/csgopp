#pragma once

#include <memory>
#include "type.h"
#include "lens.h"

namespace object
{

template<typename T>
struct Handle
{
    std::shared_ptr<const T> type;

    inline Lens view() const { return Lens(this->type); }

    inline Lens operator[](const std::string& name) const
    {
        return Lens(this->type, std::move(at(this->type.get(), name)));
    }

    inline Lens operator[](const size_t index) const
    {
        return Lens(this->type, std::move(at(this->type.get(), index)));
    }
};

}
