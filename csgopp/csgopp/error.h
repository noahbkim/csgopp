#pragma once

#include <exception>
#include <string>

namespace csgopp::error
{
    class Error : std::exception
    {
    public:
        Error() : _message() {}
        explicit Error(const std::string& message) : _message(message) {}
        explicit Error(std::string&& message) : _message(message) {}

        [[nodiscard]] const std::string& message() const
        {
            return this->_message;
        }

    private:
        std::string _message;
    };
}
