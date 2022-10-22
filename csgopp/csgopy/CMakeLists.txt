find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)

nanobind_add_module(csgopy NB_STATIC
        csgopy.h csgopy.cpp)
target_link_libraries(csgopy PRIVATE csgopp)
install(TARGETS csgopy LIBRARY DESTINATION .)
