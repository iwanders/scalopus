cmake_minimum_required(VERSION 3.5.0)

project(scalopus_examples VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

find_package(ScalopusLttng REQUIRED COMPONENTS
  scalopus_lttng)
find_package(ScalopusTransport REQUIRED COMPONENTS
  scalopus_transport)

add_executable(example_scope_tracepoints
  src/example_scope_tracepoints.cpp
)
target_link_libraries(example_scope_tracepoints scalopus_lttng scalopus_transport)


add_executable(dump_mappings src/dump_mappings.cpp)
target_link_libraries(dump_mappings scalopus_lttng scalopus_transport)
