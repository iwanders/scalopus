include (FindPkgConfig)
if (PKG_CONFIG_FOUND)
  pkg_check_modules(LTTngUST lttng-ust)
endif()

if (!LTTngUST_FOUND)
  error("LTTNG could not be found")
endif()

include(FindThreads)

add_library(scalopus_scope_tracing
  src/scope_trace_raii.cpp
  src/scope_trace_tracker.cpp
  src/endpoint_scope_tracing.cpp
)
target_include_directories(scalopus_scope_tracing
  PUBLIC 
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_library(Scalopus::scalopus_scope_tracing ALIAS scalopus_scope_tracing)

target_link_libraries(scalopus_scope_tracing
  PUBLIC
    Scalopus::scalopus_general
  PRIVATE
    nlohmann_json::nlohmann_json
)


# Create the library that provides the scope tracepoints 
add_library(scalopus_tracepoint_lttng SHARED
  src/lttng/scope_tracepoint_lttng.cpp
)
target_include_directories(scalopus_tracepoint_lttng
  PUBLIC 
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_library(Scalopus::scalopus_tracepoint_lttng ALIAS scalopus_tracepoint_lttng)

target_link_libraries(scalopus_tracepoint_lttng
  PRIVATE
    ${LTTngUST_LIBRARIES}
)

add_library(scalopus_lttng INTERFACE)
add_library(Scalopus::scalopus_lttng ALIAS scalopus_lttng)

target_link_libraries(scalopus_lttng
  INTERFACE
    Scalopus::scalopus_scope_tracing
    Scalopus::scalopus_tracepoint_lttng
)

# Create the library that provides the scope tracepoints 
add_library(scalopus_tracepoint_native SHARED
  src/native/tracepoint_collector_native.cpp
  src/native/scope_tracepoint_native.cpp
)
target_include_directories(scalopus_tracepoint_native
  PUBLIC 
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_library(Scalopus::scalopus_tracepoint_native ALIAS scalopus_tracepoint_native)

target_link_libraries(scalopus_tracepoint_native
  PRIVATE
    Scalopus::scalopus_scope_tracing
    nlohmann_json::nlohmann_json
)

add_library(scalopus_scope_tracing_native
  src/native/native_trace_endpoint_sender.cpp
)
add_library(Scalopus::scalopus_scope_tracing_native ALIAS scalopus_scope_tracing_native)

target_include_directories(scalopus_scope_tracing_native
  PUBLIC 
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(scalopus_scope_tracing_native
  PUBLIC
    Scalopus::scalopus_scope_tracing
    Scalopus::scalopus_tracepoint_native
  PRIVATE
    nlohmann_json::nlohmann_json
)

add_library(scalopus_babeltrace SHARED
  src/lttng/ctfevent.cpp
  src/lttng/babeltrace_tool.cpp
  src/lttng/babeltrace_parser.cpp)
target_link_libraries(scalopus_babeltrace
  PUBLIC
    scalopus_lttng
  PRIVATE
    Threads::Threads
)
target_include_directories(scalopus_babeltrace
  PUBLIC 
    $<INSTALL_INTERFACE:include_consumer>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include_consumer>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_library(Scalopus::scalopus_babeltrace ALIAS scalopus_babeltrace)


add_library(scalopus_lttng_consumer
  src/lttng/lttng_source.cpp
  src/lttng/lttng_provider.cpp
  src/scope_tracing_provider.cpp
  src/native/native_trace_provider.cpp
  src/native/native_trace_source.cpp
  src/native/native_trace_endpoint_receiver.cpp
)
target_include_directories(scalopus_lttng_consumer
  PUBLIC 
    $<INSTALL_INTERFACE:include_consumer>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include_consumer>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_library(Scalopus::scalopus_lttng_consumer ALIAS scalopus_lttng_consumer)
target_link_libraries(scalopus_lttng_consumer
  PUBLIC
    scalopus_lttng
    scalopus_babeltrace
    Scalopus::scalopus_consumer
)

export(
  TARGETS
    scalopus_scope_tracing
    scalopus_babeltrace
    scalopus_lttng
    scalopus_lttng_consumer
    scalopus_tracepoint_lttng
    scalopus_tracepoint_native
    scalopus_scope_tracing_native
  NAMESPACE Scalopus::
  FILE ${SCALOPUS_COMPONENT_CMAKE_CONFIG_DIR}/ScalopusTracingConfig.cmake
)
install(TARGETS
  scalopus_lttng
  scalopus_babeltrace
  scalopus_lttng_consumer
  scalopus_scope_tracing
  scalopus_tracepoint_lttng
  scalopus_tracepoint_native
  scalopus_scope_tracing_native

  EXPORT ScalopusTracing
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ScalopusTracing DESTINATION share/scalopus_tracing/cmake)

add_subdirectory(test)

