
set(PROTO_OUTDIR ${CMAKE_CURRENT_BINARY_DIR})
set(PROTO_FILES
    urpc/protocol/urpc/urpc_meta.proto)
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
    urpc/urpc.cc
    urpc/transport.cc
    urpc/endpoint.cc
    urpc/connect_transport.cc
    urpc/client_transport.cc
    urpc/server_transport.cc
    urpc/controller.cc
    urpc/client_call.cc
    urpc/server_call.cc
    urpc/acceptor.cc
    urpc/poller.cc
    urpc/epoll.cc
    urpc/channel.cc
    urpc/iobuf.cc
    urpc/io_context.cc
    urpc/server.cc
    urpc/service_holder.cc

    urpc/protocol/manager.cc
    urpc/protocol/urpc/call.cc
    urpc/protocol/urpc/protocol.cc
    )

add_library(urpc STATIC ${FILES})
target_include_directories(urpc PUBLIC "${PROJECT_SOURCE_DIR}/include"
    PRIVATE "${PROJECT_SOURCE_DIR}/src")
target_link_libraries(urpc pthread glog protobuf::libprotobuf gflags urpc_proto)
# target_link_libraries(urpc
#     -Wl,--whole-archive
#     urpc_protocol
#     -Wl,--no-whole-archive)
target_compile_options(urpc PRIVATE -g)
