cmake_minimum_required(VERSION 3.9)
project(toucan)

include(cmake/BuildTypes.cmake)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -g)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "bin")

add_subdirectory(libs)

add_subdirectory(toucan)

option(TOUCAN_TESTS "Enable toucan library tests" OFF)

if(${TOUCAN_TESTS})
    enable_testing()
    add_subdirectory(tests)
endif()

