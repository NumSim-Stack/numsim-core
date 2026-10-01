# numsim_add_test (numsim-cmake): the executable plus ASan+UBSan variant,
# both registered with ctest. Options after the sources are passed through
# (TSAN, NO_MAIN, ...).
macro(add_numsim_core_test TARGET_NAME)
    numsim_add_test(${TARGET_NAME} SOURCES ${ARGN} LINK numsim-core::numsim-core)
endmacro()

# Iterate over all example subdirectories
file(GLOB TEST_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

foreach(TEST_DIRS ${TEST_DIRS})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_DIRS})
        add_subdirectory(${TEST_DIRS})
    endif()
endforeach()
