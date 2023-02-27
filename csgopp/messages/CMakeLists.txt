set(Protobuf_PROTOC_EXECUTABLE ${CONAN_BIN_DIRS_PROTOBUF}/protoc.exe)

find_package(Protobuf REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})

protobuf_generate_cpp(PROTO_SOURCES PROTO_HEADERS
        cstrike15_gcmessages.proto
        cstrike15_usermessages.proto
        engine_gcmessages.proto
        netmessages.proto
        steammessages.proto)

set_property(TARGET protobuf::protoc PROPERTY IMPORTED_LOCATION ${CONAN_BIN_DIRS_PROTOBUF}/protoc.exe)

# https://stackoverflow.com/questions/20824194/cmake-with-google-protocol-buffers
add_library(csgopp.messages STATIC ${PROTO_SOURCES} ${PROTO_HEADERS})
target_include_directories(csgopp.messages
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
        PRIVATE ${Protobuf_LIBRARIES})
