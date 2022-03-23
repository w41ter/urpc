
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

    protocol/manager.cc
    protocol/echo/call.cc
    protocol/echo/protocol.cc
    )

add_library(urpc ${FILES})
target_include_directories(urpc PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(urpc pthread glog protobuf::libprotobuf gflags)
