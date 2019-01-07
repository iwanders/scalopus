cmake_minimum_required(VERSION 3.5.0)

find_package(ScalopusTransport REQUIRED)
find_package(ScalopusLttng REQUIRED)

add_executable(example_scope_tracepoints
  src/example_scope_tracepoints.cpp
)
target_link_libraries(example_scope_tracepoints Scalopus::scalopus_lttng)

add_executable(dump_mappings src/dump_mappings.cpp)
target_link_libraries(dump_mappings Scalopus::scalopus_lttng)

add_executable(example_scope_tracepoints_random
  src/example_scope_tracepoints_random.cpp
)
target_link_libraries(example_scope_tracepoints_random Scalopus::scalopus_lttng)