#include "server_class.h"

namespace csgopp::client::server_class
{

std::string join(const std::string& head, const std::string& tail, bool skip)
{
    if (tail.empty() || tail == "baseclass" || skip)
    {
        return head;
    }
    else if (head.empty())
    {
        return tail;
    }
    else
    {
        return head + "." + tail;
    }
}

}
