macro(add_numsim_core_example TARGET_NAME)
    add_executable(${TARGET_NAME} ${ARGN})
    target_link_libraries(${TARGET_NAME} PRIVATE numsim-core::numsim-core)
    #maybe_target_pedantic_warnings(${TARGET_NAME})
endmacro()

# Iterate over all example subdirectories
file(GLOB EXAMPLE_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*)

foreach(EXAMPLE_DIR ${EXAMPLE_DIRS})
    if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_DIR})
        add_subdirectory(${EXAMPLE_DIR})
    endif()
endforeach()
