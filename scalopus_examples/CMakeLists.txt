cmake_minimum_required(VERSION 3.5.0)

# Examples defaulting to native tracepoints.
add_executable(readme_example_native src/readme_example.cpp)
target_link_libraries(readme_example_native Scalopus::scalopus_tracing_native)
target_compile_options(readme_example_native PRIVATE ${SCALOPUS_COMPILE_OPTIONS})

add_executable(random_callstacks_native src/random_callstacks.cpp)
target_compile_options(random_callstacks_native PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
target_link_libraries(random_callstacks_native Scalopus::scalopus_tracing_native)

# Examples defaulting to nop tracepoints.
add_executable(readme_example_nop src/readme_example.cpp)
target_link_libraries(readme_example_nop Scalopus::scalopus_tracing_nop)
target_compile_options(readme_example_nop PRIVATE ${SCALOPUS_COMPILE_OPTIONS})

add_executable(random_callstacks_nop src/random_callstacks.cpp)
target_compile_options(random_callstacks_nop PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
target_link_libraries(random_callstacks_nop Scalopus::scalopus_tracing_nop)

# Examples defaulting to lttng tracepoints.
if(TARGET Scalopus::scalopus_tracing_lttng)
  add_executable(readme_example_lttng src/readme_example.cpp)
  target_compile_options(readme_example_lttng PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
  target_link_libraries(readme_example_lttng Scalopus::scalopus_tracing_lttng)

  add_executable(random_callstacks_lttng src/random_callstacks.cpp)
  target_compile_options(random_callstacks_lttng PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
  target_link_libraries(random_callstacks_lttng Scalopus::scalopus_tracing_lttng)

  add_executable(showcase_marker_events_lttng src/showcase_marker_events.cpp)
  target_compile_options(showcase_marker_events_lttng PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
  target_link_libraries(showcase_marker_events_lttng Scalopus::scalopus_tracing_lttng)
endif()

# Example that tries to retrieve as much information from discovered servers as possible.
add_executable(query_servers src/query_servers.cpp)
target_compile_options(query_servers PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
target_link_libraries(query_servers Scalopus::scalopus_transport Scalopus::scalopus_general_consumer Scalopus::scalopus_tracing_consumer)

# Example from the quickstart, basically it is identical to readme_example_native.
add_executable(readme_example src/readme_example.cpp)
target_compile_options(readme_example PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
target_link_libraries(readme_example Scalopus::scalopus_tracing_native)

# Example which is kinda odd, it contains the catapult server in the same process using the loopback interface.
add_executable(embedded_catapult_server src/embedded_catapult_server.cpp)
target_compile_options(embedded_catapult_server PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
target_link_libraries(embedded_catapult_server Scalopus::scalopus_catapult Scalopus::scalopus_tracing_native)

# Showcase example for enable and disabling tracepoints.
add_executable(showcase_toggle_tracepoints src/showcase_toggle_tracepoints.cpp)
target_compile_options(showcase_toggle_tracepoints PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
target_link_libraries(showcase_toggle_tracepoints Scalopus::scalopus_tracing_native)

# Showcase example for the marker events
add_executable(showcase_marker_events src/showcase_marker_events.cpp)
target_compile_options(showcase_marker_events PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
target_link_libraries(showcase_marker_events Scalopus::scalopus_tracing_native)

# Showcase example for the count events
add_executable(showcase_counters src/showcase_counters.cpp)
target_compile_options(showcase_counters PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
target_link_libraries(showcase_counters Scalopus::scalopus_tracing_native)
