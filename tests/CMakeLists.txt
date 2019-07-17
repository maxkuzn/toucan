add_subdirectory("${PROJECT_SOURCE_DIR}/libs/googletest" "libs/googletest")

macro(package_add_test TESTNAME)
    add_executable(${TESTNAME} ${ARGN})
    target_link_libraries(${TESTNAME} toucan gtest gmock gtest_main)
    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
    set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
endmacro()

file(GLOB TEST_SRC_FILES "test_*.cpp")

foreach(TEST_SRC_FILE ${TEST_SRC_FILES})
    get_filename_component(TEST_NAME ${TEST_SRC_FILE} NAME_WE)
    package_add_test(${TEST_NAME} ${TEST_SRC_FILE})
endforeach()
