
set(FILES
    urpc.cc)

add_library(urpc ${FILES})
target_include_directories(urpc PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(urpc pthread glog)
