find_package(Protobuf REQUIRED)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")
PROTOBUF_GENERATE_CPP(PROTO_SOURCES PROTO_HEADERS ${PROTO_FILES})
add_library(csgopp.msg STATIC ${PROTO_SOURCES} ${PROTO_HEADERS})

# https://stackoverflow.com/questions/20824194/cmake-with-google-protocol-buffers
set(PROTO_INCLUDE ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "Path to generated protobuf files.")
