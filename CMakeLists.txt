cmake_minimum_required(VERSION 3.5.0)

project(Scalopus VERSION 0.0.1)

# Include libraries first.
option(SCALOPUS_USE_EXTERNAL_JSON "Obtain nlohmann_json through find_package." OFF)
option(SCALOPUS_USE_EXTERNAL_SEASOCKS "Obtain Seasocks through find_package." OFF)
option(SCALOPUS_USE_EXTERNAL_PYBIND "Try to obtain pybind11 through find_package." OFF)
option(SCALOPUS_BUILD_TRACING_LTTNG "Try to build the LTTNG tracing target." ON)

# Allow quick configuration of the Python version. For manually specfiying the version use PYBIND11_PYTHON_VERSION.
option(SCALOPUS_USE_PYTHON2 "Set PYBIND11_PYTHON_VERSION to 2." OFF)
option(SCALOPUS_USE_PYTHON3 "Set PYBIND11_PYTHON_VERSION to 3." OFF)


set(SCALOPUS_TRACING_HAVE_BUILT_LTTNG OFF)

include(GNUInstallDirs)
set(SCALOPUS_INSTALL_EXPORT_CMAKE_DIR  "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}/")
set(SCALOPUS_EXPORT_CMAKE_DIR "${CMAKE_CURRENT_BINARY_DIR}/")

# We need an include here to ensure that the find package call originates from the root.
# add_subdirectory won't work as that won't expose embedded targets to the scalopus subcomponent directories.
# Include allows handling of the 3rd party libraries to be done in a centralized manner and ensures that the
# subcomponents can assume that the targets exist.
include(thirdparty/CMakeLists.txt)


# Set properties for the remainder of the builds
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CLANG_ONLY_COMPILE_FLAGS "-Wconversion;")
  message(STATUS "Compiler is clang, adding options: ${CLANG_ONLY_COMPILE_FLAGS}")
  # disable mismatched tags until https://github.com/nlohmann/json/issues/1401
  set(CLANG_ONLY_COMPILE_FLAGS "${CLANG_ONLY_COMPILE_FLAGS};-Wno-mismatched-tags")
endif()
set(SCALOPUS_COMPILE_OPTIONS "-Wall;-Wextra;-Wshadow;-Wnon-virtual-dtor;-Wpedantic;${CLANG_ONLY_COMPILE_FLAGS}")

include(FindThreads)

include(CTest)
if(BUILD_TESTING)
  enable_testing()
endif()

# Each 'submodule' acts as a completely seperate cmake package. They all export a `ScalopusInterfaceConfig.cmake` file
# the main `ScalopusConfig.cmake` uses find_package() on the others to find them.
add_subdirectory(scalopus_interface)
add_subdirectory(scalopus_transport)
add_subdirectory(scalopus_tracing)
add_subdirectory(scalopus_general)
add_subdirectory(scalopus_examples)
add_subdirectory(scalopus_catapult)
add_subdirectory(scalopus_python)

file(GLOB_RECURSE FORMAT_SRC_FILES  "${PROJECT_SOURCE_DIR}/scalopus*/**.h"  "${PROJECT_SOURCE_DIR}/scalopus*/**.cpp")
add_custom_target(clang_format_scalopus COMMAND clang-format-7 -i ${FORMAT_SRC_FILES})

# Write version file
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

# Write package config file
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ScalopusConfig.cmake.in
    ${SCALOPUS_EXPORT_CMAKE_DIR}/${PROJECT_NAME}Config.cmake
    INSTALL_DESTINATION ${SCALOPUS_INSTALL_EXPORT_CMAKE_DIR})

# Install both files.
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION ${SCALOPUS_INSTALL_EXPORT_CMAKE_DIR}
)
