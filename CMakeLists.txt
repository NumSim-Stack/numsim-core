cmake_minimum_required(VERSION 3.25)

project(numsim-core LANGUAGES CXX)

# Set project include directory
set(${PROJECT_NAME}_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME})

# Default C++ standard for examples / tests / anything in this tree that
# doesn't pick its own. The library's own C++23 requirement is declared
# below as an INTERFACE compile feature on the target itself, so consumers
# inherit it through the linked target — independently of this default.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include meta information
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/project-meta-info.in")
    include("${CMAKE_CURRENT_LIST_DIR}/project-meta-info.in")
else()
    message(WARNING "Meta info file not found: ${CMAKE_CURRENT_LIST_DIR}/project-meta-info.in")
endif()

# Project version
set(${PROJECT_NAME}_VERSION ${project_version})
message(STATUS "Building v${${PROJECT_NAME}_VERSION}")

# Header files

# numsim-core is header-only: no sources, no compiled artefacts, just an
# INTERFACE target carrying include paths + a C++23 feature requirement
# that propagates to every consumer.
#
# Previously declared as STATIC with the headers listed as sources, plus
# `target_compile_features(... PRIVATE cxx_std_20)`. Both were bugs:
#   - STATIC with header-only "sources" produces an empty archive.
#   - PRIVATE on a header-only target doesn't propagate, so consumers
#     compiled with whatever standard they happened to have — and broke
#     on `<print>` from input_parameter_controller.h.
add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# GNUInstallDirs BEFORE the target_include_directories() below, not after it.
# CMAKE_INSTALL_INCLUDEDIR is expanded when the line is read, so with the
# include further down the INSTALL_INTERFACE expanded EMPTY and the exported
# target carried no include directory at all -- consumers of the installed
# package could find it and not compile against it. An empty INSTALL_INTERFACE
# is legal, so nothing warned.
include(GNUInstallDirs)

# Headers are reachable from the build tree and from the install tree.
target_include_directories(${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# C++23 propagates via INTERFACE — every consumer that links
# numsim-core::numsim-core inherits the requirement and gets compiled
# with at least C++23. This is what makes `#include <print>` work in
# downstream TUs (rvegen, Tessera, etc.).
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)

# Build options
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(NUMSIM_CORE_IS_TOP_LEVEL ON)
else()
    set(NUMSIM_CORE_IS_TOP_LEVEL OFF)
endif()
option(NUMSIM_CORE_INSTALL         "Install the numsim-core library"  ON)
option(NUMSIM_CORE_BUILD_TESTS     "Build the numsim-core test suite" ${NUMSIM_CORE_IS_TOP_LEVEL})
option(NUMSIM_CORE_BUILD_EXAMPLES  "Build the numsim-core examples"   ${NUMSIM_CORE_IS_TOP_LEVEL})
option(NUMSIM_CORE_BUILD_BENCHMARK "Build the numsim-core benchmarks" OFF)

include(cmake/numsim_bootstrap.cmake)
include(NumSimDependency)
include(NumSimWarnings)

# Installation logic
if(NUMSIM_CORE_INSTALL)
    install(
        TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    # Install header files
    # install(DIRECTORY), not install(FILES): FILES flattens everything into
    # one directory, so include/numsim-core/property_graph/*.h landed beside
    # the top-level headers and #include <numsim-core/property_graph/...>
    # could not resolve. It also only installed the files listed in
    # a hand-maintained list -- 13 of the 23 headers in the tree.
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
            FILES_MATCHING PATTERN "*.h")

    # Create and install package config files
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        VERSION ${${PROJECT_NAME}_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
        "${CMAKE_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake
    )

    install(
        FILES "${CMAKE_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
              "${CMAKE_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake
    )

    install(
        EXPORT ${PROJECT_NAME}_Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake
    )
endif()

# Optional subdirectories
if(NUMSIM_CORE_BUILD_TESTS)
    enable_testing()
    include(NumSimGoogleTest)
    numsim_googletest()
    include(NumSimTesting)
    add_subdirectory(tests)
endif()

if(NUMSIM_CORE_BUILD_BENCHMARK)
    # add_subdirectory(benchmark)
endif()

if(NUMSIM_CORE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# CPack configuration
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${${PROJECT_NAME}_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "NumSim Core Library")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
include(CPack)
