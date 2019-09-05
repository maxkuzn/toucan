add_subdirectory("${PROJECT_SOURCE_DIR}/libs/googletest" "libs/googletest")

macro(package_add_test TEST_NAME)
    add_executable(${TEST_NAME} ${ARGN})
    target_link_libraries(${TEST_NAME} toucan gtest gtest_main)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    set_target_properties(${TEST_NAME} PROPERTIES FOLDER tests)
endmacro()

file(GLOB TEST_SRC_FILES "test_*.cpp")

foreach(TEST_SRC_FILE ${TEST_SRC_FILES})
    get_filename_component(TEST_NAME ${TEST_SRC_FILE} NAME_WE)
    package_add_test(${TEST_NAME} ${TEST_SRC_FILE})
endforeach()

file(GLOB SRCS *.cpp)
add_executable(all_tests ${SRCS})
target_link_libraries(all_tests toucan gtest)
add_test(NAME all_tests COMMAND all_tests)
set_target_properties(all_tests PROPERTIES FOLDER tests)

