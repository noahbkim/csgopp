add_library(csgopp STATIC
        source/csgopp/client.cpp
        source/csgopp/client/data_table.cpp
        source/csgopp/client/data_table/data_property.cpp
        source/csgopp/client/data_table/data_table_property.cpp
        source/csgopp/client/data_table/data_type.cpp
        source/csgopp/client/entity.cpp
        source/csgopp/client/game_event.cpp
        source/csgopp/client/server_class.cpp
        source/csgopp/common/control.cpp
        source/csgopp/demo.cpp
        )

add_subdirectory(docs)
add_subdirectory(messages)
add_subdirectory(tests)

set_target_properties(csgopp PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_features(csgopp PRIVATE cxx_std_20)

target_include_directories(csgopp PUBLIC include)
target_link_libraries(csgopp
        PUBLIC object
        PUBLIC csgopp.messages
        PUBLIC CONAN_PKG::abseil
        PUBLIC CONAN_PKG::protobuf)
