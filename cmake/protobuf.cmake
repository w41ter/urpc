set(protobuf_INSTALL OFF)
set(protobuf_BUILD_TESTS OFF)

FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG "v3.19.4"
    SOURCE_SUBDIR  cmake
)
FetchContent_MakeAvailable(protobuf)
set(PROTOBUF_PROTOC_EXECUTABLE ${protobuf_BINARY_DIR}/protoc)