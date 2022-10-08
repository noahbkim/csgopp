add_library(csgopp STATIC
        demo.cpp demo.h
        common/lookup.h
        game.cpp game.h
        common/macro.h
        game/team.cpp game/team.h
        game/player.cpp game/player.h
        common/vector.h
        network/entity.cpp network/entity.h
        common/id.h
        common/interface.h
        network/server_class.cpp network/server_class.h
        error.h
        network/data_table.cpp network/data_table.h network.h network/declare.h network/data_table/property.h network/data_table/property.cpp network/data_table/value.cpp network/data_table/value.h network.cpp common/magic.h common/bits.h network/string_table.cpp network/string_table.h common/ring.h)

set_target_properties(csgopp PROPERTIES POSITION_INDEPENDENT_CODE ON)

include_directories(${PROTO_INCLUDE})

target_include_directories(csgopp PUBLIC ..)
target_compile_features(csgopp PRIVATE cxx_std_17)
target_link_libraries(csgopp csgopp.msg ${CONAN_LIBS})
