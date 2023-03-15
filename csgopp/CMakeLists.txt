add_library(csgopp STATIC
        source/client.cpp
        source/client/data_table.cpp
        source/client/data_table/data_property.cpp
        source/client/data_table/data_table_property.cpp
        source/client/data_table/data_type.cpp
        source/client/entity.cpp
        source/client/game_event.cpp
        source/client/server_class.cpp
        source/common/control.cpp
        source/demo.cpp
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
