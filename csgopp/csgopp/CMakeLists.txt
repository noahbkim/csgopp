add_library(csgopp STATIC
        client.cpp client.h
        client/data_table.cpp
        client/data_table.h
        client/data_table/data_property.cpp
        client/data_table/data_property.h
        client/data_table/data_table_property.cpp
        client/data_table/data_table_property.h
        client/data_table/data_type.cpp
        client/data_table/data_type.h
        client/data_table/property.h
        client/entity.cpp
        client/entity.h
        client/game_event.cpp
        client/game_event.h
        client/server_class.h
        client/string_table.h
        client/user.h
        common/bits.h
        common/control.cpp
        common/control.h
        common/database.h
        common/id.h
        common/lookup.h
        common/macro.h
        common/reader.h
        common/ring.h
        common/vector.h
        demo.cpp
        demo.h
        error.h
        client/server_class.cpp)

set_target_properties(csgopp PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_features(csgopp PRIVATE cxx_std_20)

target_include_directories(csgopp PUBLIC ..)
target_link_libraries(csgopp
        PUBLIC csgopp.messages object
        PUBLIC CONAN_PKG::abseil CONAN_PKG::protobuf)
