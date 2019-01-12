cmake_minimum_required(VERSION 3.5.0)

project(Scalopus VERSION 0.0.1)

# Include libraries first.
option(SCALOPUS_USE_EXTERNAL_JSON "Obtain nlohmann_json through find_package." OFF)
option(SCALOPUS_USE_EXTERNAL_SEASOCKS "Obtain Seasocks through find_package." OFF)

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

add_subdirectory(scalopus_interface)
add_subdirectory(scalopus_transport)
add_subdirectory(scalopus_lttng)
add_subdirectory(scalopus_general)
add_subdirectory(scalopus_examples)
add_subdirectory(scalopus_catapult)
