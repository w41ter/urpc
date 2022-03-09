enable_testing()

set(PROTO_OUTDIR ${CMAKE_CURRENT_BINARY_DIR})
set(PROTO_FILES echo.proto)
foreach(P ${PROTO_FILES})
    string(REPLACE .proto .pb.h HDR ${P})
    string(REPLACE .proto .pb.cc SRC ${P})
    add_custom_command(
        OUTPUT ${PROTO_OUTDIR}/${HDR} ${PROTO_OUTDIR}/${SRC}
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} ${PROTOC_FLAGS} -I${CMAKE_CURRENT_SOURCE_DIR} --cpp_out=${PROTO_OUTDIR} ${P}
        DEPENDS ${P}
    )
endforeach()

add_library(test_proto ${PROTO_OUTDIR}/echo.pb.cc)
target_include_directories(test_proto PUBLIC ${PROTO_OUTDIR}/)
target_link_libraries(test_proto protobuf::libprotobuf)

function(urpc_test TEST_FILE)
    get_filename_component(TARGET_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TARGET_NAME} ${TEST_FILE})
    target_link_libraries(${TARGET_NAME} PRIVATE test_proto urpc gtest_main)
    add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
endfunction()

urpc_test(client_transport_test.cc)
urpc_test(echo_test.cc)
urpc_test(server_test.cc)
