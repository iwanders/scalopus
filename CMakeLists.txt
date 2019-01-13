cmake_minimum_required(VERSION 3.5.0)

project(Scalopus VERSION 0.0.1)

# Include libraries first.
option(SCALOPUS_USE_EXTERNAL_JSON "Obtain nlohmann_json through find_package." OFF)
option(SCALOPUS_USE_EXTERNAL_SEASOCKS "Obtain Seasocks through find_package." OFF)
set(SCALOPUS_CMAKE_CONFIG_DIR          "${CMAKE_CURRENT_BINARY_DIR}")
set(SCALOPUS_COMPONENT_CMAKE_CONFIG_DIR          "${CMAKE_CURRENT_BINARY_DIR}/cmake/")
set(SCALOPUS_CMAKE_PROJECT_CONFIG_FILE "${SCALOPUS_CMAKE_CONFIG_DIR}/${PROJECT_NAME}Config.cmake")
set(SCALOPUS_CONFIG_INSTALL_DIR        "cmake/${PROJECT_NAME}" CACHE INTERNAL "")

# We need an include here to ensure that the find package call originates from the root.
# add_subdirectory won't work as that won't expose embedded targets to the scalopus subcomponent directories.
# Include allows handling of the 3rd party libraries to be done in a centralized manner and ensures that the
# subcomponents can assume that the targets exist.
include(thirdparty/CMakeLists.txt)


# Set properties for the remainder of the builds
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(CLANG_ONLY_COMPILE_FLAGS "-Wconversion;")
  message(STATUS "Compiler is clang, adding options: ${CLANG_ONLY_COMPILE_FLAGS}")
endif()

add_compile_options("-Werror;-Wall;-Wextra;-Wshadow;-Wnon-virtual-dtor;-Wpedantic;${CLANG_ONLY_COMPILE_FLAGS}")

include(GNUInstallDirs)
include(FindThreads)

include(CTest)
if(BUILD_TESTING)
  enable_testing()
endif()

# Each 'submodule' acts as a completely seperate cmake package basically, they only thing they lack is find_package()
# to each other. This ensures that any externally created provider / source has the exact same capabilities as the
# ones inside this cmake project.
add_subdirectory(scalopus_interface)
add_subdirectory(scalopus_transport)
add_subdirectory(scalopus_tracing)
add_subdirectory(scalopus_general)
add_subdirectory(scalopus_examples)
add_subdirectory(scalopus_catapult)


configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/ScalopusConfig.cmake.in
  ${SCALOPUS_CMAKE_PROJECT_CONFIG_FILE}
  @ONLY
)

install(
  FILES ${SCALOPUS_CMAKE_PROJECT_CONFIG_FILE}
  DESTINATION ${SCALOPUS_CONFIG_INSTALL_DIR}
)