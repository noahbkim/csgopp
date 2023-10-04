include_directories(${PROTO_INCLUDE})

add_executable(csgod source/main.cpp)
target_include_directories(csgod PRIVATE include)

find_package(absl REQUIRED)
find_package(argparse REQUIRED)
target_link_libraries(csgod
        PRIVATE csgopp
        PRIVATE abseil::abseil
        PRIVATE argparse::argparse
        )
