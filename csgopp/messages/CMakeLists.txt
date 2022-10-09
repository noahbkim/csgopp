find_package(Protobuf REQUIRED)

PROTOBUF_GENERATE_CPP(PROTO_SOURCES PROTO_HEADERS
        cstrike15_gcmessages.proto
        cstrike15_usermessages.proto
        engine_gcmessages.proto
        netmessages.proto
        steammessages.proto)
add_library(csgopp.messages STATIC ${PROTO_SOURCES} ${PROTO_HEADERS})

# https://stackoverflow.com/questions/20824194/cmake-with-google-protocol-buffers
target_include_directories(csgopp.messages PUBLIC ${PROTO_INCLUDE})
