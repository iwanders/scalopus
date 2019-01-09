cmake_minimum_required(VERSION 3.5.0)

find_package(ScalopusTransport REQUIRED)
find_package(ScalopusGeneral REQUIRED)

include (FindPkgConfig)
if (PKG_CONFIG_FOUND)
  pkg_check_modules(LTTngUST lttng-ust)
endif()

find_package(nlohmann_json 3.2.0 REQUIRED)

if (!LTTngUST_FOUND)
  error("LTTNG could not be found")
endif()

include(FindThreads)

# Create the library that provides the scope tracepoints 
add_library(scalopus_lttng SHARED
  src/scope_tracepoint.cpp
  src/scope_trace_raii.cpp
  src/scope_trace_tracker.cpp
  src/endpoint_scope_tracing.cpp
)

target_include_directories(scalopus_lttng
  PUBLIC 
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
add_library(Scalopus::scalopus_lttng ALIAS scalopus_lttng)

target_link_libraries(scalopus_lttng
  PUBLIC
    Scalopus::scalopus_transport
    Scalopus::scalopus_interface
    Scalopus::scalopus_general
  PRIVATE
    nlohmann_json::nlohmann_json
    ${LTTngUST_LIBRARIES}
)


add_library(scalopus_babeltrace SHARED
  src/ctfevent.cpp
  src/babeltrace_tool.cpp
  src/babeltrace_parser.cpp)
target_link_libraries(scalopus_babeltrace
  PUBLIC
    scalopus_lttng
  PRIVATE
    Threads::Threads
)
add_library(Scalopus::scalopus_babeltrace ALIAS scalopus_babeltrace)


add_executable(babeltrace_test
  src/babeltrace_test.cpp)
target_link_libraries(babeltrace_test
  PRIVATE
    scalopus_babeltrace
)

add_library(scalopus_lttng_consumer SHARED
  src/lttng_source.cpp
  src/lttng_provider.cpp
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
  TARGETS scalopus_lttng scalopus_babeltrace scalopus_lttng_consumer
  NAMESPACE Scalopus::
  FILE ScalopusLttngConfig.cmake
)
install(TARGETS scalopus_lttng scalopus_babeltrace scalopus_lttng_consumer EXPORT ScalopusLttng
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT ScalopusLttng DESTINATION share/scalopus_lttng/cmake)

add_subdirectory(test)

