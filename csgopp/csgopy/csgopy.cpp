#include <functional>

#include "csgopy.h"

NB_MODULE(csgopy, m) {
    m.def("call", [](const nb::object& f) { f(); }, "f"_a);
}
