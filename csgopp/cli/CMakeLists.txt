include_directories(${PROTO_INCLUDE})

add_executable(csgopp.cli main.cpp info.h generate.h common.h advance.h)
target_link_libraries(csgopp.cli
        PUBLIC CONAN_PKG::argparse
        PUBLIC csgopp)
