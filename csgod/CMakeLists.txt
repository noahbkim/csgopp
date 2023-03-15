include_directories(${PROTO_INCLUDE})

add_executable(csgod source/main.cpp)
target_include_directories(csgod PRIVATE include)

target_link_libraries(csgod
        PUBLIC csgopp
        PUBLIC CONAN_PKG::argparse
        )
