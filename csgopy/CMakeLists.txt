if (MSVC)
    find_package(Python COMPONENTS Interpreter Development Development.Module REQUIRED)
else ()
    find_package(Python3 COMPONENTS Interpreter Development Development.Module REQUIRED)
endif ()

nanobind_add_module(csgopy NB_STATIC
        source/csgopy/client.cpp
        source/csgopy/common.cpp
        source/csgopy/data_table.cpp
        source/csgopy/demo.cpp
        source/csgopy/entity.cpp
        source/csgopy/game_event.cpp
        source/csgopy/object.cpp
        source/csgopy/server_class.cpp
        source/csgopy/string_table.cpp
        source/csgopy/user.cpp
        source/csgopy/main.cpp)

target_link_libraries(csgopy PRIVATE csgopp)
install(TARGETS csgopy LIBRARY DESTINATION .)
