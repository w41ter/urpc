
set(PROTO_OUTDIR ${CMAKE_CURRENT_BINARY_DIR})
set(PROTO_FILES
    protocol/urpc/urpc_meta.proto)
foreach(P ${PROTO_FILES})
    string(REPLACE .proto .pb.h HDR ${P})
    string(REPLACE .proto .pb.cc SRC ${P})
    add_custom_command(
        OUTPUT ${PROTO_OUTDIR}/${HDR} ${PROTO_OUTDIR}/${SRC}
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} ${PROTOC_FLAGS} -I${CMAKE_CURRENT_SOURCE_DIR} --cpp_out=${PROTO_OUTDIR} ${P}
        DEPENDS ${P}
    )
endforeach()

add_library(urpc_proto STATIC ${PROTO_OUTDIR}/protocol/urpc/urpc_meta.pb.cc)
target_include_directories(urpc_proto PUBLIC
    ${PROTO_OUTDIR}
    ${PROTO_OUTDIR}/protocol/urpc)
target_link_libraries(urpc_proto protobuf::libprotobuf)

# set(PROTOCOL_FILES
#     )
# add_library(urpc_protocol STATIC ${PROTOCOL_FILES})
# target_include_directories(urpc_protocol PUBLIC "${PROJECT_SOURCE_DIR}/include")
# target_link_libraries(urpc_protocol pthread glog protobuf::libprotobuf gflags urpc_proto)
# target_compile_options(urpc_protocol PRIVATE -g)

set(FILES
    urpc.cc
    transport.cc
    endpoint.cc
    connect_transport.cc
    client_transport.cc
    server_transport.cc
    controller.cc
    client_call.cc
    server_call.cc
    acceptor.cc
    poller.cc
    epoll.cc
    channel.cc
    iobuf.cc
    io_context.cc
    server.cc
    service_holder.cc

    protocol/manager.cc
    protocol/urpc/call.cc
    protocol/urpc/protocol.cc
    )

add_library(urpc STATIC ${FILES})
target_include_directories(urpc PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(urpc pthread glog protobuf::libprotobuf gflags urpc_proto)
# target_link_libraries(urpc
#     -Wl,--whole-archive
#     urpc_protocol
#     -Wl,--no-whole-archive)
target_compile_options(urpc PRIVATE -g)
