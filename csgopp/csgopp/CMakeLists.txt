add_library(csgopp STATIC
        demo.cpp demo.h
        common/lookup.h
        client.cpp client.h
        common/macro.h
        common/vector.h
        client/entity.cpp client/entity.h
        common/id.h
        client/server_class.cpp client/server_class.h
        error.h
        client/data_table.cpp client/data_table.h
        common/bits.h
        client/string_table.cpp client/string_table.h
        common/ring.h
        common/reader.h common/database.h common/control.h common/object.h
        common/object.h common/object.cpp
        client/user.h client/user.cpp common/code.h common/code.cpp common/control.cpp common/layout.h client/game_event.h client/game_event.cpp)

set_target_properties(csgopp PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_features(csgopp PRIVATE cxx_std_17)

target_include_directories(csgopp PUBLIC ..)
target_link_libraries(csgopp
        PUBLIC csgopp.messages
        PUBLIC CONAN_PKG::abseil CONAN_PKG::protobuf)
