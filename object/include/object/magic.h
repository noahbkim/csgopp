#include <type_traits>

namespace object
{

template<typename T>
struct dependent_false : std::false_type {};

}
