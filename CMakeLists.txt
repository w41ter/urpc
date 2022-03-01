cmake_minimum_required(VERSION 3.10)
project(urpc
    VERSION 0.1.0
    DESCRIPTION "An RPC framework under linux platform"
    LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-fPIC)

include(cmake/FetchThirdParty.cmake)
add_subdirectory(src)
