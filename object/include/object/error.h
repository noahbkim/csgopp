#pragma

#include <stdexcept>

namespace object
{

struct Error : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

struct MemberError : public Error
{
    using Error::Error;
};

struct IndexError : public Error
{
    using Error::Error;
};

struct TypeError : public Error
{
    using Error::Error;
};

}
