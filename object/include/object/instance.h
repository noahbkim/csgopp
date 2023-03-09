#pragma once

#include <memory>
#include <string_view>
#include "object.h"

namespace object
{

struct Instance
{
    virtual ~Instance() {}

    [[nodiscard]] virtual std::shared_ptr<const Type> type() const = 0;
    [[nodiscard]] virtual char* address() = 0;
    [[nodiscard]] virtual const char* address() const = 0;
};

}
