macro(add_numsim_core_test TARGET_NAME)
    add_executable(${TARGET_NAME} ${ARGN})
    target_link_libraries(${TARGET_NAME} PRIVATE numsim-core::numsim-core GTest::gtest_main)
    #maybe_target_pedantic_warnings(${TARGET_NAME})
    gtest_discover_tests(${TARGET_NAME})  # Automatically discover tests for the executable
    add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})
endmacro()

# Iterate over all example subdirectories
file(GLOB TEST_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

foreach(TEST_DIRS ${TEST_DIRS})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_DIRS})
        add_subdirectory(${TEST_DIRS})
    endif()
endforeach()
