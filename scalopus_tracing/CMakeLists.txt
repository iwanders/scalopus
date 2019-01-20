set(SCALOPUS_TRACING_EXPORTS "")

include(FindThreads)

add_library(scalopus_scope_tracing
  src/scope_trace_raii.cpp
  src/scope_trace_tracker.cpp
  src/endpoint_trace_mapping.cpp
)
target_include_directories(scalopus_scope_tracing
  PUBLIC 
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_options(scalopus_scope_tracing PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
add_library(Scalopus::scalopus_scope_tracing ALIAS scalopus_scope_tracing)

target_link_libraries(scalopus_scope_tracing
  PUBLIC
    Scalopus::scalopus_general
  PRIVATE
    nlohmann_json::nlohmann_json
)
list(APPEND SCALOPUS_TRACING_EXPORTS scalopus_scope_tracing)

# Conditionally build the tracing_lttng target.
if (SCALOPUS_BUILD_TRACING_LTTNG)

  include (FindPkgConfig)
  if (PKG_CONFIG_FOUND)
    pkg_check_modules(LTTngUST lttng-ust)
  endif()

  if (LTTngUST_FOUND)
    # Create the library that provides the scope tracepoints 
    add_library(scalopus_tracepoint_lttng SHARED
      src/lttng/scope_tracepoint_lttng.cpp
    )
    target_compile_options(scalopus_tracepoint_lttng PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
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
    list(APPEND SCALOPUS_TRACING_EXPORTS scalopus_tracepoint_lttng)

    add_library(scalopus_tracing_lttng INTERFACE)
    add_library(Scalopus::scalopus_tracing_lttng ALIAS scalopus_tracing_lttng)

    target_link_libraries(scalopus_tracing_lttng
      INTERFACE
        Scalopus::scalopus_scope_tracing
        Scalopus::scalopus_tracepoint_lttng
    )
    list(APPEND SCALOPUS_TRACING_EXPORTS scalopus_tracing_lttng)
  else()
    message(WARNING "Could not find lttng-ust this build will NOT provide the scalopus_tracing_lttng target.")
  endif()
else()
    message(WARNING "Building scalopus_tracing_lttng is disabled via SCALOPUS_BUILD_TRACING_LTTNG")
endif()

# Create the library that provides the scope tracepoints 
add_library(scalopus_tracepoint_native SHARED
  src/native/tracepoint_collector_native.cpp
  src/native/scope_tracepoint_native.cpp
)
target_compile_options(scalopus_tracepoint_native PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
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
list(APPEND SCALOPUS_TRACING_EXPORTS scalopus_tracepoint_native)

add_library(scalopus_tracing_native
  src/native/endpoint_native_trace_sender.cpp
)
target_compile_options(scalopus_tracing_native PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
add_library(Scalopus::scalopus_tracing_native ALIAS scalopus_tracing_native)

target_include_directories(scalopus_tracing_native
  PUBLIC 
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(scalopus_tracing_native
  PUBLIC
    Scalopus::scalopus_scope_tracing
    Scalopus::scalopus_tracepoint_native
  PRIVATE
    nlohmann_json::nlohmann_json
)
list(APPEND SCALOPUS_TRACING_EXPORTS scalopus_tracing_native)

add_library(scalopus_lttng_babeltrace SHARED
  src/lttng/ctfevent.cpp
  src/lttng/babeltrace_tool.cpp
  src/lttng/babeltrace_parser.cpp)
target_compile_options(scalopus_lttng_babeltrace PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
target_link_libraries(scalopus_lttng_babeltrace
  PRIVATE
    Threads::Threads
)
target_include_directories(scalopus_lttng_babeltrace
  PUBLIC 
    $<INSTALL_INTERFACE:include_consumer>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include_consumer>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_library(Scalopus::scalopus_lttng_babeltrace ALIAS scalopus_lttng_babeltrace)
list(APPEND SCALOPUS_TRACING_EXPORTS scalopus_lttng_babeltrace)

# The lttng providers don't actually depend on lttng-ust, so we can always build them.
add_library(scalopus_tracing_consumer
  src/lttng/lttng_source.cpp
  src/lttng/lttng_provider.cpp
  src/scope_tracing_provider.cpp
  src/native/native_trace_provider.cpp
  src/native/native_trace_source.cpp
  src/native/endpoint_native_trace_receiver.cpp
)
target_compile_options(scalopus_tracing_consumer PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
target_include_directories(scalopus_tracing_consumer
  PUBLIC 
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include_consumer>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include_consumer>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_library(Scalopus::scalopus_tracing_consumer ALIAS scalopus_tracing_consumer)
target_link_libraries(scalopus_tracing_consumer
  PRIVATE
    Scalopus::scalopus_scope_tracing
  PUBLIC
    Scalopus::scalopus_general
    Scalopus::scalopus_lttng_babeltrace
    Scalopus::scalopus_consumer
)
list(APPEND SCALOPUS_TRACING_EXPORTS scalopus_tracing_consumer)

export(
  TARGETS
    ${SCALOPUS_TRACING_EXPORTS}
  NAMESPACE Scalopus::
  FILE ${SCALOPUS_COMPONENT_CMAKE_CONFIG_DIR}/ScalopusTracingConfig.cmake
)
install(TARGETS
    ${SCALOPUS_TRACING_EXPORTS}
  EXPORT ScalopusTracing
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ScalopusTracing DESTINATION share/scalopus_tracing/cmake)

add_subdirectory(test)

