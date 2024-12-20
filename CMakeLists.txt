cmake_minimum_required(VERSION 3.15...3.24)

project(numsim-core LANGUAGES CXX)

include(FetchContent)

# Set project include directory
set(${PROJECT_NAME}_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME})

# Set C++ standard requirements
set(CMAKE_CXX_STANDARD 20)
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
set(${PROJECT_NAME}_HEADER
    ${${PROJECT_NAME}_INCLUDE_DIR}/factory_base_bones.h
    ${${PROJECT_NAME}_INCLUDE_DIR}/factory_base_meat.h
    ${${PROJECT_NAME}_INCLUDE_DIR}/registry_bones.h
    ${${PROJECT_NAME}_INCLUDE_DIR}/registry_meat.h
    ${${PROJECT_NAME}_INCLUDE_DIR}/warehouse_bones.h
    ${${PROJECT_NAME}_INCLUDE_DIR}/warehouse_meat.h
    ${${PROJECT_NAME}_INCLUDE_DIR}/input_parameter_controller.h
    ${${PROJECT_NAME}_INCLUDE_DIR}/input_parser.h
    ${${PROJECT_NAME}_INCLUDE_DIR}/static_indexing.h
    ${${PROJECT_NAME}_INCLUDE_DIR}/numsim_core_utility.h
    ${${PROJECT_NAME}_INCLUDE_DIR}/query_map.h
    ${${PROJECT_NAME}_INCLUDE_DIR}/parameter_handler.h
    ${${PROJECT_NAME}_INCLUDE_DIR}/any_printer.h
    ${${PROJECT_NAME}_INCLUDE_DIR}/wrapper.h
)

# Add the library
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_HEADER})

# Explicitly set the linker language
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# Add alias for easier linking
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# Include directories
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
          $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# Enable required C++ features
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# Build options
option(${PROJECT_NAME}_INSTALL_LIBRARY "Enable installing of ${PROJECT_NAME} library" ON)
option(BUILD_EXAMPLES "Build ${PROJECT_NAME} examples" ON)
option(BUILD_TESTS "Build ${PROJECT_NAME} test suite" ON)
option(BUILD_BENCHMARK "Build ${PROJECT_NAME} benchmarks" OFF)
option(DOWNLOAD_GTEST "Download and build GTest" OFF)
option(DOWNLOAD_GBENCHMARK "Download and build Google Benchmark" OFF)

# Installation logic
include(GNUInstallDirs)
if(${PROJECT_NAME}_INSTALL_LIBRARY)
    install(
        TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    # Install header files
    install(FILES ${${PROJECT_NAME}_HEADER}
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

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
if(BUILD_TESTS)
    enable_testing()
    include(CTest)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    include(GoogleTest)
    # Uncomment when tests exist
    add_subdirectory(tests)
endif()

if(BUILD_BENCHMARK)
    if(DOWNLOAD_GBENCHMARK)
        # Add logic to download and build Google Benchmark here
    endif()
    # Uncomment when benchmarks exist
    # add_subdirectory(benchmark)
endif()

if(BUILD_EXAMPLES)
    # Uncomment when examples exist
    add_subdirectory(examples)
endif()

# CPack configuration
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${${PROJECT_NAME}_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "NumSim Core Library")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
include(CPack)
