find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

PROTOBUF_GENERATE_CPP(PROTO_SOURCES PROTO_HEADERS
        cstrike15_gcmessages.proto
        cstrike15_usermessages.proto
        engine_gcmessages.proto
        netmessages.proto
        steammessages.proto)

# https://stackoverflow.com/questions/20824194/cmake-with-google-protocol-buffers
add_library(csgopp.messages STATIC ${PROTO_SOURCES} ${PROTO_HEADERS})
target_include_directories(csgopp.messages
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
        PRIVATE CONAN_PKG::protobuf)
