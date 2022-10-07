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
        network/send_table.cpp network/send_table.h)

set_target_properties(csgopp PROPERTIES POSITION_INDEPENDENT_CODE ON)

include_directories(${PROTO_INCLUDE})

target_include_directories(csgopp PUBLIC ..)
target_compile_features(csgopp PRIVATE cxx_std_17)
target_link_libraries(csgopp csgopp.msg ${CONAN_LIBS})
