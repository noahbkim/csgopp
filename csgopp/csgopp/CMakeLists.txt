add_library(csgopp STATIC
        demo.cpp demo.h
        common/lookup.h
        game.cpp game.h
        common/reader.h
        game/team.cpp game/team.h
        game/player.cpp game/player.h
        common/vector.h
        game/entity.cpp game/entity.h
        common/id.h common/interface.h game/server_class.cpp game/server_class.h error.h game/send_table.cpp game/send_table.h)

include_directories(${PROTO_INCLUDE})

target_include_directories(csgopp PUBLIC ..)
target_compile_features(csgopp PRIVATE cxx_std_17)
target_link_libraries(csgopp csgopp.msg ${CONAN_LIBS})
