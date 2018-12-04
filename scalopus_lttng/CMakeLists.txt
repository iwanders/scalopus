cmake_minimum_required(VERSION 3.5.0)

include (FindPkgConfig)
if (PKG_CONFIG_FOUND)
  pkg_check_modules(LTTngUST lttng-ust)
endif()

if (!LTTngUST_FOUND)
  error("LTTNG could not be found")
endif()

# Create the library that provides the scope tracepoints 
add_library(scalopus_lttng SHARED
  src/scope_tracepoint.cpp
  src/scope_trace_raii.cpp
  src/scope_trace_tracker.cpp
  src/endpoint_scope_tracing.cpp
  src/client_scope_tracing.cpp
)

target_include_directories(scalopus_lttng
  PUBLIC 
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_library(Scalopus::Lttng ALIAS scalopus_lttng)

target_link_libraries(scalopus_lttng
  PUBLIC
    Scalopus::Transport
  PRIVATE
    ${LTTngUST_LIBRARIES}
)


