cmake_minimum_required(VERSION 3.5.0)

add_executable(example_scope_tracepoints
  src/example_scope_tracepoints.cpp
)
target_link_libraries(example_scope_tracepoints Scalopus::Lttng)

add_executable(dump_mappings src/dump_mappings.cpp)
target_link_libraries(dump_mappings Scalopus::Lttng)
