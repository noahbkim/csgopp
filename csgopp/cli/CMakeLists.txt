include_directories(${PROTO_INCLUDE})

add_executable(csgopp.cli main.cpp info.h)
target_link_libraries(csgopp.cli csgopp)
