if (MSVC)
    find_package(Python COMPONENTS Interpreter Development Development.Module REQUIRED)
else ()
    find_package(Python3 COMPONENTS Interpreter Development Development.Module REQUIRED)
endif ()

nanobind_add_module(csgopy NB_STATIC
        csgopy.h csgopy.cpp)

target_link_libraries(csgopy PRIVATE csgopp)
install(TARGETS csgopy LIBRARY DESTINATION .)
