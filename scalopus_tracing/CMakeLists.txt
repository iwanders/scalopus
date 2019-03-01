set(SCALOPUS_TRACING_EXPORTS "")

include(FindThreads)

add_library(scalopus_scope_tracing SHARED
  src/scope_trace_tracker.cpp
  src/endpoint_trace_mapping.cpp
  src/trace_configurator.cpp
  src/trace_configuration_raii.cpp
  src/native/tracepoint_collector_native.cpp
  src/native/endpoint_native_trace_sender.cpp
)
set_property(TARGET scalopus_scope_tracing PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(scalopus_scope_tracing
  PUBLIC 
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/scalopus_tracing/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
target_compile_options(scalopus_scope_tracing PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
target_compile_features(scalopus_scope_tracing PUBLIC cxx_relaxed_constexpr)
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
      src/lttng/lttng_tracepoint.cpp
    )
    target_compile_options(scalopus_tracepoint_lttng PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
    target_include_directories(scalopus_tracepoint_lttng
      PUBLIC 
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/scalopus_tracing/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/lttng/include>
      PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    )
    add_library(Scalopus::scalopus_tracepoint_lttng ALIAS scalopus_tracepoint_lttng)

    target_link_libraries(scalopus_tracepoint_lttng
      PRIVATE
        ${LTTngUST_LIBRARIES}
    )
    list(APPEND SCALOPUS_TRACING_EXPORTS scalopus_tracepoint_lttng)

    add_library(scalopus_tracing_lttng SHARED
      src/lttng/scope_tracepoint_lttng.cpp
      src/scope_trace_raii.cpp)
    add_library(Scalopus::scalopus_tracing_lttng ALIAS scalopus_tracing_lttng)
    target_include_directories(scalopus_tracing_lttng
      PUBLIC 
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/scalopus_tracing/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    )

    target_link_libraries(scalopus_tracing_lttng
      PUBLIC
        Scalopus::scalopus_scope_tracing
        Scalopus::scalopus_tracepoint_lttng
    )
    list(APPEND SCALOPUS_TRACING_EXPORTS scalopus_tracing_lttng)
    set(SCALOPUS_TRACING_HAVE_BUILT_LTTNG ON PARENT_SCOPE)
  else()
    message(WARNING "Could not find lttng-ust this build will NOT provide the scalopus_tracing_lttng target.")
  endif()
else()
    message(WARNING "Building scalopus_tracing_lttng is disabled via SCALOPUS_BUILD_TRACING_LTTNG")
endif()

# Create the library that provides the scope tracepoints 
add_library(scalopus_tracepoint_native SHARED
  src/native/native_tracepoint.cpp
)
target_compile_options(scalopus_tracepoint_native PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
target_include_directories(scalopus_tracepoint_native
  PUBLIC 
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/scalopus_tracing/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/native/include>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
add_library(Scalopus::scalopus_tracepoint_native ALIAS scalopus_tracepoint_native)

target_link_libraries(scalopus_tracepoint_native
  PRIVATE
    Scalopus::scalopus_scope_tracing
    nlohmann_json::nlohmann_json
)
list(APPEND SCALOPUS_TRACING_EXPORTS scalopus_tracepoint_native)

add_library(scalopus_tracing_native SHARED
  src/scope_trace_raii.cpp
  src/native/scope_tracepoint_native.cpp
)
add_library(Scalopus::scalopus_tracing_native ALIAS scalopus_tracing_native)
target_link_libraries(scalopus_tracing_native
  PUBLIC
    Scalopus::scalopus_tracepoint_native
    Scalopus::scalopus_scope_tracing
)
target_include_directories(scalopus_tracing_native
  PUBLIC 
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/scalopus_tracing/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
list(APPEND SCALOPUS_TRACING_EXPORTS scalopus_tracing_native)

add_library(scalopus_tracepoint_nop SHARED
  src/nop/nop_tracepoint.cpp
)
target_compile_options(scalopus_tracepoint_nop PRIVATE ${SCALOPUS_COMPILE_OPTIONS})
target_include_directories(scalopus_tracepoint_nop
  PUBLIC 
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/scalopus_tracing/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/nop/include>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
add_library(Scalopus::scalopus_tracepoint_nop ALIAS scalopus_tracepoint_nop)

list(APPEND SCALOPUS_TRACING_EXPORTS scalopus_tracepoint_nop)

add_library(scalopus_tracing_nop SHARED
  src/scope_trace_raii.cpp
  src/nop/scope_tracepoint_nop.cpp
)
add_library(Scalopus::scalopus_tracing_nop ALIAS scalopus_tracing_nop)
target_link_libraries(scalopus_tracing_nop
  PUBLIC
    Scalopus::scalopus_tracepoint_nop
    Scalopus::scalopus_scope_tracing
)
target_include_directories(scalopus_tracing_nop
  PUBLIC 
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/scalopus_tracing/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
)
list(APPEND SCALOPUS_TRACING_EXPORTS scalopus_tracing_nop)


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
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/scalopus_tracing/include_consumer>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include_consumer>
  PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
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
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/scalopus_tracing/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include_consumer>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_library(Scalopus::scalopus_tracing_consumer ALIAS scalopus_tracing_consumer)
target_link_libraries(scalopus_tracing_consumer
  PUBLIC
    Scalopus::scalopus_scope_tracing
    Scalopus::scalopus_general
    Scalopus::scalopus_lttng_babeltrace
    Scalopus::scalopus_consumer
)
set_property(TARGET scalopus_tracing_consumer PROPERTY POSITION_INDEPENDENT_CODE ON)
list(APPEND SCALOPUS_TRACING_EXPORTS scalopus_tracing_consumer)

export(
  TARGETS
    ${SCALOPUS_TRACING_EXPORTS}
  NAMESPACE Scalopus::
  FILE ${SCALOPUS_EXPORT_CMAKE_DIR}/ScalopusTracingConfig.cmake
)
install(TARGETS
    ${SCALOPUS_TRACING_EXPORTS}
  EXPORT ScalopusTracingConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/scalopus_tracing/)
install(DIRECTORY include_consumer DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/scalopus_tracing/)
install(DIRECTORY src/nop/include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/scalopus_tracing/)
install(DIRECTORY src/lttng/include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/scalopus_tracing/)
install(DIRECTORY src/native/include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/scalopus_tracing/)
install(
  EXPORT ScalopusTracingConfig
  FILE ScalopusTracingConfig.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${SCALOPUS_INSTALL_EXPORT_CMAKE_DIR}
)
add_subdirectory(test)

