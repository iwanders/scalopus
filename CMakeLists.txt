cmake_minimum_required(VERSION 3.5.0)

project(Scalopus VERSION 0.0.1)

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

# https://youtu.be/bsXLMQ6WgIk?t=3170
set(as_subproject ScalopusTransport ScalopusLttng ScalopusGeneral)
macro(find_package)
  if(NOT "${ARGV0}" IN_LIST as_subproject)
    _find_package(${ARGV})
  else()
    message("Detected ${ARGV0} as submodule.")
  endif()
endmacro()

include(CTest)
if(BUILD_TESTING)
  enable_testing()
endif()

add_subdirectory(scalopus_transport)
add_subdirectory(scalopus_lttng)
add_subdirectory(scalopus_examples)
add_subdirectory(scalopus_catapult)
add_subdirectory(scalopus_general)
