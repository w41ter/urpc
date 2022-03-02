enable_testing()

function(urpc_test TEST_FILE)
    get_filename_component(TARGET_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TARGET_NAME} ${TEST_FILE})
    target_link_libraries(${TARGET_NAME} PRIVATE urpc gtest_main)
    add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
endfunction()

urpc_test(client_transport_test.cc)
