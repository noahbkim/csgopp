find_package(Python COMPONENTS Interpreter Development Development.Module REQUIRED)

nanobind_add_module(csgopy NB_STATIC
        source/csgopy/csgopp/client/client.cpp
        source/csgopy/csgopp/common/vector.cpp
        source/csgopy/csgopp/client/data_table.cpp
        source/csgopy/csgopp/client/demo.cpp
        source/csgopy/csgopp/client/entity.cpp
        source/csgopy/csgopp/client/game_event.cpp
        source/csgopy/objective/object.cpp
        source/csgopy/csgopp/client/server_class.cpp
        source/csgopy/csgopp/client/string_table.cpp
        source/csgopy/csgopp/client/user.cpp
        source/csgopy/main.cpp)

target_include_directories(csgopy PRIVATE include)
target_link_libraries(csgopy PRIVATE csgopp)
install(TARGETS csgopy LIBRARY DESTINATION .)
